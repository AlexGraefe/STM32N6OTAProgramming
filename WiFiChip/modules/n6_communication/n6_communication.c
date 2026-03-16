#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

#include "n6_communication.h"
#include "tcp_socket.h"


#define SPIOP  (SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_OP_MODE_SLAVE)  // SPI_MODE_CPOL | SPI_MODE_CPHA

#define SPI_RETRY_COUNT UINT16_MAX

const struct device *spi_dev = DEVICE_DT_GET(SPI_NODE);
static struct spi_config spi_cfg = {
    .frequency = DT_PROP(SPI_NODE, clock_frequency),
    .operation = SPIOP,
    .slave = 0,
    .cs = {
        .gpio = {NULL}, // No GPIO for CS
        .delay = 0,     // No delay
    },
};

K_SEM_DEFINE(spi_sem, 0, 1);
static volatile int spi_async_result;

LOG_MODULE_REGISTER(n6, LOG_LEVEL_DBG);

K_THREAD_DEFINE(n6_com_thread, CONFIG_N6_COMMUNICATION_THREAD_STACK_SIZE,
                run_n6_communication, NULL, NULL, NULL,
                N6_COMMUNICATION_THREAD_PRIORITY, 0, 0);

static int spi_handshake(void)
{
	int ret;

	if (!device_is_ready(spi_dev)) {
		LOG_ERR("SPI device not ready");
		return -ENODEV;
	}
	
	LOG_DBG("Starting SPI handshake with master...");

	uint8_t tx_handshake[4] = {255, 254, 253, 252};
	uint8_t rx_handshake[4];

	struct spi_buf tx_buf = {
		.buf = (void *)tx_handshake,
		.len = sizeof(tx_handshake),
	};
	struct spi_buf_set tx_bufs = {
		.buffers = &tx_buf,
		.count = 1,
	};
	struct spi_buf rx_buf = {
		.buf = rx_handshake,
		.len = sizeof(tx_handshake),
	};
	struct spi_buf_set rx_bufs = {
		.buffers = &rx_buf,
		.count = 1,
	};

	for (int attempt = 1; attempt <= SPI_RETRY_COUNT; attempt++) {
		memset(rx_handshake, 0, sizeof(tx_handshake));
		ret = spi_transceive(spi_dev, &spi_cfg, &tx_bufs, &rx_bufs);
		if (ret < 0) {
			LOG_WRN("SPI transceive attempt %d/%d failed (%d)",
				attempt, SPI_RETRY_COUNT, ret);
			continue;
		}

		if (memcmp(tx_handshake, rx_handshake, sizeof(tx_handshake)) == 0) {
			LOG_INF("SPI handshake completed");
			return 0;
		}

		LOG_WRN("SPI verify mismatch on attempt %d/%d",
			attempt, SPI_RETRY_COUNT);
	}

	LOG_ERR("SPI handshake failed after retries");
	return -EIO;
}

static void spi_rx_cb(const struct device *dev, int result, void *data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(data);

	spi_async_result = result;

	if (result < 0) {
		LOG_ERR("SPI async transceive failed with result %d", result);
 	}

	k_sem_give(&spi_sem);
}


static int sending_messages(void)
{
	int ret;
	struct k_msgq *socket_packet_msgq = get_socket_packet_msgq();
	struct k_msgq *socket_spi_ack_msgq = get_socket_spi_ack_msgq();

	for (;;) {
		iris_packet_t packet;
		uint8_t rx_discard[sizeof(iris_packet_t)] = {0};
		struct spi_buf tx_buf_data = {
			.buf = &packet,
			.len = sizeof(packet),
		};
		struct spi_buf_set tx_bufs_data = {
			.buffers = &tx_buf_data,
			.count = 1,
		};
		struct spi_buf rx_buf_data = {
			.buf = rx_discard,
			.len = sizeof(rx_discard),
		};
		struct spi_buf_set rx_bufs_data = {
			.buffers = &rx_buf_data,
			.count = 1,
		};

		ret = k_msgq_get(socket_packet_msgq, &packet, K_FOREVER);
		if (ret < 0) {
			LOG_ERR("Failed to dequeue packet from socket queue (%d)", ret);
			return ret;
		}

		spi_async_result = 0;
		ret = spi_transceive_cb(spi_dev, &spi_cfg, &tx_bufs_data, &rx_bufs_data, spi_rx_cb, NULL);

		if (ret < 0) {
			LOG_ERR("SPI write data submit failed (%d)", ret);
			return ret;
		}

		if (k_sem_take(&spi_sem, K_FOREVER) != 0) {
			LOG_ERR("SPI completion semaphore wait failed");
			return -EIO;
		}

		if (spi_async_result < 0) {
			LOG_ERR("SPI write data failed (%d)", spi_async_result);
			return spi_async_result;
		}

		socket_spi_ack_message_t ack_message = {
			.packet_idx = packet.packet_idx,
			.status = 0,
		};
		ret = k_msgq_put(socket_spi_ack_msgq, &ack_message, K_FOREVER);
		if (ret < 0) {
			LOG_ERR("Failed to enqueue SPI ACK for packet=%u (%d)", packet.packet_idx, ret);
			return ret;
		}

		LOG_DBG("Sent SPI packet: packet=%u",
			packet.packet_idx);
	}
	return 0;
}

int run_n6_communication(void)
{
	/* Wait till tcp socket is set up */
	socket_start_message_t message;
	struct k_msgq *socket_start_msgq = get_socket_start_msgq();

	k_msgq_get(socket_start_msgq, &message, K_FOREVER);
		
	/* Do handshake with spi master */ 
	if (spi_handshake() != 0) {
		return -1;
	}

	LOG_INF("Starting to forward TCP data over SPI...");
	return sending_messages();

}
