#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/util.h>

#include "wifi_utilities.h"
#include "secret/wifi_pswd.h"
#include "zephyr/net/net_ip.h"
#include "tcp_socket.h"

#include <zephyr/logging/log.h>

static const struct gpio_dt_spec led_red = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led_green = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led_blue = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

K_MSGQ_DEFINE(socket_start_msgq, sizeof(socket_start_message_t), 2, 1);
K_MSGQ_DEFINE(socket_packet_msgq, sizeof(iris_packet_t), 8, 4);
K_MSGQ_DEFINE(socket_spi_ack_msgq, sizeof(socket_spi_ack_message_t), 1, 4);

#define LED_TURN_OFF() do { gpio_pin_set_dt(&led_red, 0); gpio_pin_set_dt(&led_green, 0); gpio_pin_set_dt(&led_blue, 0); } while(0)
#define LED_TURN_RED() do { gpio_pin_set_dt(&led_red, 1); gpio_pin_set_dt(&led_green, 0); gpio_pin_set_dt(&led_blue, 0); } while(0)
#define LED_TURN_GREEN() do { gpio_pin_set_dt(&led_red, 0); gpio_pin_set_dt(&led_green, 1); gpio_pin_set_dt(&led_blue, 0); } while(0)
#define LED_TURN_BLUE() do { gpio_pin_set_dt(&led_red, 0); gpio_pin_set_dt(&led_green, 0); gpio_pin_set_dt(&led_blue, 1); } while(0)
#define LED_TURN_YELLOW() do { gpio_pin_set_dt(&led_red, 1); gpio_pin_set_dt(&led_green, 1); gpio_pin_set_dt(&led_blue, 0); } while(0)
#define LED_TURN_CYAN() do { gpio_pin_set_dt(&led_red, 0); gpio_pin_set_dt(&led_green, 1); gpio_pin_set_dt(&led_blue, 1); } while(0)
#define LED_TURN_MAGENTA() do { gpio_pin_set_dt(&led_red, 1); gpio_pin_set_dt(&led_green, 0); gpio_pin_set_dt(&led_blue, 1); } while(0)
#define LED_TURN_WHITE() do { gpio_pin_set_dt(&led_red, 1); gpio_pin_set_dt(&led_green, 1); gpio_pin_set_dt(&led_blue, 1); } while(0)


LOG_MODULE_REGISTER(tcp_socket_demo, LOG_LEVEL_DBG);

K_THREAD_DEFINE(tcp_thread, CONFIG_TCP_SOCKET_THREAD_STACK_SIZE,
				run_tcp_socket_demo, NULL, NULL, NULL,
                SOCKET_THREAD_PRIORITY, 0, 0);

static int recv_exact(int sock_fd, uint8_t *buffer, size_t size)
{
	size_t received = 0U;

	while (received < size) {
		int ret = zsock_recv(sock_fd, &buffer[received], size - received, 0);

		if (ret == 0) {
			return -ECONNRESET;
		}

		if (ret < 0) {
			return -errno;
		}

		received += (size_t)ret;
	}

	return 0;
}

static const char *state_to_string(communication_state_t state)
{
	switch (state) {
	case COMM_WIFI_CONNECTING:
		return "COMM_WIFI_CONNECTING";
	case COMM_WAITING_FOR_IP:
		return "COMM_WAITING_FOR_IP";
	case COMM_ESTABLISHING_SERVER:
		return "COMM_CONNECTING_TO_SERVER";
	case COMM_RECEIVING_MESSAGES:
		return "COMM_RECEIVING_MESSAGES";
	case COMM_FAILURE:
		return "COMM_FAILURE";
	case COMM_CLEANUP:
		return "COMM_CLEANUP";
	case COMM_DONE:
		return "COMM_DONE";
	default:
		return "COMM_UNKNOWN";
	}
}

static communication_state_t state_wifi_connecting(communication_context_t *ctx)
{
	LED_TURN_MAGENTA();
	if (my_wifi_init() != 0) {
		LOG_ERR("Failed to initialize WiFi module");
		ctx->failure_from_state = COMM_WIFI_CONNECTING;
		return COMM_FAILURE;
	}

	LOG_INF("Connecting to WiFi...");

	if (wifi_connect(BITCRAZE_SSID, BITCRAZE_PASSWORD)) {
		LOG_ERR("Failed to connect to WiFi");
		ctx->failure_from_state = COMM_WIFI_CONNECTING;
		return COMM_FAILURE;
	}

	ctx->wifi_connected = true;
	return COMM_WAITING_FOR_IP;
}

static communication_state_t state_waiting_for_ip(communication_context_t *ctx)
{
	LED_TURN_CYAN();

	if (wifi_wait_for_ip_addr(ctx->ip_addr) != 0) {
		LOG_ERR("Failed while waiting for IPv4 address");
		ctx->failure_from_state = COMM_WAITING_FOR_IP;
		return COMM_FAILURE;
	}

	return COMM_ESTABLISHING_SERVER;
}

static communication_state_t state_connecting_to_server(communication_context_t *ctx)
{
	int ret;

	LED_TURN_YELLOW();

	ctx->sock_fd = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ctx->sock_fd < 0) {
		LOG_ERR("Could not create socket (errno=%d)", errno);
		ctx->failure_from_state = COMM_ESTABLISHING_SERVER;
		return COMM_FAILURE;
	}
	ctx->socket_open = true;

	memset(&ctx->server_addr, 0, sizeof(ctx->server_addr));
	ctx->server_addr.sin_family = AF_INET;
	ctx->server_addr.sin_port = htons(SERVER_PORT);

	ret = zsock_inet_pton(AF_INET, SERVER_IP, &ctx->server_addr.sin_addr);
	if (ret != 1) {
		LOG_ERR("Invalid SERVER_IP (%s)", SERVER_IP);
		ctx->failure_from_state = COMM_ESTABLISHING_SERVER;
		return COMM_FAILURE;
	}

	ret = zsock_connect(ctx->sock_fd, (struct sockaddr *)&ctx->server_addr, sizeof(ctx->server_addr));
	if (ret < 0) {
		LOG_ERR("Could not connect to server (errno=%d)", errno);
		ctx->failure_from_state = COMM_ESTABLISHING_SERVER;
		return COMM_FAILURE;
	}

	LOG_INF("[Client] Connected to %s:%d", SERVER_IP, SERVER_PORT);

	k_msgq_purge(&socket_packet_msgq);
	k_msgq_purge(&socket_spi_ack_msgq);

	socket_start_message_t message;
	message.enabled = 1;
	k_msgq_put(&socket_start_msgq, &message, K_NO_WAIT);

	return COMM_RECEIVING_MESSAGES;
}

static communication_state_t state_receiving_messages(communication_context_t *ctx)
{
	ctx->failure_from_state = COMM_RECEIVING_MESSAGES;

	for (;;) {
		iris_packet_t packet;
		int ret;

		LED_TURN_GREEN();
		ret = recv_exact(ctx->sock_fd, (uint8_t *)&packet, sizeof(packet));
		if (ret < 0) {
			LOG_ERR("TCP receive failed (%d)", ret);
			return COMM_FAILURE;
		}

		ret = k_msgq_put(&socket_packet_msgq, &packet, K_FOREVER);
		if (ret < 0) {
			LOG_ERR("Failed to enqueue packet packet=%u (%d)", packet.packet_idx, ret);
			return COMM_FAILURE;
		}

		socket_spi_ack_message_t ack_message;
		ret = k_msgq_get(&socket_spi_ack_msgq, &ack_message, K_FOREVER);
		if (ret < 0) {
			LOG_ERR("Failed waiting SPI ACK for packet=%u (%d)", packet.packet_idx, ret);
			return COMM_FAILURE;
		}

		if ((ack_message.status != 0) || (ack_message.packet_idx != packet.packet_idx)) {
			LOG_ERR("Invalid SPI ACK: expected=%u got=%u status=%d",
				packet.packet_idx,
				ack_message.packet_idx,
				ack_message.status);
			return COMM_FAILURE;
		}

		LED_TURN_RED();
	}

	return COMM_CLEANUP;
}

static communication_state_t state_failure(communication_context_t *ctx)
{
	LOG_ERR("[Failure] Called from: %s", state_to_string(ctx->failure_from_state));
	LOG_ERR("[Failure] Context: sock_fd=%d socket_open=%d wifi_connected=%d exit_code=%d",
			ctx->sock_fd,
			ctx->socket_open,
			ctx->wifi_connected,
			ctx->exit_code);

	ctx->exit_code = -1;
	while (1) {
		/* Unique failure indication: blinking red */
		LED_TURN_RED();
		k_sleep(K_SECONDS(1));
		LED_TURN_OFF();
		k_sleep(K_SECONDS(1));
	}
	return COMM_CLEANUP;
}

static communication_state_t state_cleanup(communication_context_t *ctx)
{
	/* Unique cleanup indication: blinking blue */
	for (int i = 0; i < 3; i++) {
		LED_TURN_BLUE();
		k_sleep(K_MSEC(150));
		LED_TURN_OFF();
		k_sleep(K_MSEC(150));
	}

	if (ctx->socket_open) {
		zsock_close(ctx->sock_fd);
		ctx->socket_open = false;
		LOG_INF("[Server] Closed");
	}

	if (ctx->wifi_connected) {
		wifi_disconnect();
		ctx->wifi_connected = false;
	}

	return COMM_DONE;
}

int run_tcp_socket_demo(void)
{
	communication_state_t state = COMM_WIFI_CONNECTING;

	/* static so context does not live on the thread stack */
	static communication_context_t ctx = {
		.sock_fd = -1,
		.wifi_connected = false,
		.socket_open = false,
		.exit_code = 0,
		.failure_from_state = COMM_FAILURE,
	};

	int ret = gpio_pin_configure_dt(&led_red, GPIO_OUTPUT_INACTIVE);
    ret |= gpio_pin_configure_dt(&led_green, GPIO_OUTPUT_INACTIVE);
    ret |= gpio_pin_configure_dt(&led_blue, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        state = COMM_FAILURE;
    }
	LED_TURN_OFF();

	LOG_INF("TCP -> SPI STREAMER");

	while (state != COMM_DONE) {
		LOG_DBG("State: %s", state_to_string(state));
		switch (state) {
		case COMM_WIFI_CONNECTING:
			state = state_wifi_connecting(&ctx);
			break;
		case COMM_WAITING_FOR_IP:
			state = state_waiting_for_ip(&ctx);
			break;
		case COMM_ESTABLISHING_SERVER:
			state = state_connecting_to_server(&ctx);
			break;
		case COMM_RECEIVING_MESSAGES:
			state = state_receiving_messages(&ctx);
			break;
		case COMM_FAILURE:
			state = state_failure(&ctx);
			break;
		case COMM_CLEANUP:
			state = state_cleanup(&ctx);
			break;
		case COMM_DONE:
			break;
		default:
			ctx.failure_from_state = state;
			state = COMM_FAILURE;
			break;
		}
	}
	LED_TURN_OFF();

	return ctx.exit_code;
}

struct k_msgq *get_socket_packet_msgq(void)
{
	return &socket_packet_msgq;
}

struct k_msgq *get_socket_start_msgq(void)
{
	return &socket_start_msgq;
}

struct k_msgq *get_socket_spi_ack_msgq(void)
{
	return &socket_spi_ack_msgq;
}
