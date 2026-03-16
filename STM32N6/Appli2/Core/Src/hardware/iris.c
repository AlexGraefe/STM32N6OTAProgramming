#include "iris.h"
#include "stm32_hal_legacy.h"
#include "stm32n6xx.h"
#include "stm32n6xx_hal.h"
#include "stm32n6xx_hal_gpio.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "timer.h"
#include "app_config.h"

static SPI_HandleTypeDef hspi5;
static const uint8_t iris_handshake_expected[4] = {255, 254, 253, 252};
static uint32_t iris_frame_nmbr = 0U;

static void iris_transmit_raw(const uint8_t *data, uint16_t size)
{
    HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_RESET);
    my_sleep(100);
    HAL_SPI_Transmit(&hspi5, (uint8_t *)data, size, ~0);
    my_sleep(100);
    HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_SET);
}


/**
  * @brief SPI MSP Initialization
  * This function configures the hardware resources used
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if(hspi->Instance==SPI5)
    {


        /** Initializes the peripherals clock
        */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI5;
        PeriphClkInitStruct.Spi5ClockSelection = RCC_SPI5CLKSOURCE_PCLK2;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            while (1) {}
        }

        /* Peripheral clock enable */
        __HAL_RCC_SPI5_CLK_ENABLE();

        __HAL_RCC_GPIOE_CLK_ENABLE();
        __HAL_RCC_GPIOH_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOG_CLK_ENABLE();
        /**SPI5 GPIO Configuration
        PE15     ------> SPI5_SCK
        PH8     ------> SPI5_MISO
        PG2     ------> SPI5_MOSI
        PA3     ------> SPI5_NSS
        */
        GPIO_InitStruct.Pin = GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
        HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
        HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = CS_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(CS_GPIO_PORT, &GPIO_InitStruct);

        printf("gggggg\n");

        /* USER CODE BEGIN SPI5_MspInit 1 */

        /* USER CODE END SPI5_MspInit 1 */
    }

}

void iris_config()
{
    hspi5.Instance = SPI5;
    hspi5.Init.Mode = SPI_MODE_MASTER;
    hspi5.Init.Direction = SPI_DIRECTION_2LINES;
    hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi5.Init.NSS = SPI_NSS_SOFT;
    hspi5.Init.BaudRatePrescaler = APP_IRIS_SPI_BAUDRATE_PRESCALER;
    hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi5.Init.CRCPolynomial = 0x7;
    hspi5.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;    // SPI_NSS_PULSE_ENABLE
    hspi5.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    hspi5.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    hspi5.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    hspi5.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    hspi5.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    hspi5.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    hspi5.Init.IOSwap = SPI_IO_SWAP_DISABLE;
    hspi5.Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY;
    hspi5.Init.ReadyPolarity = SPI_RDY_POLARITY_HIGH;
    if (HAL_SPI_Init(&hspi5) != HAL_OK)
    {
        while(1) {}
    }
}

void iris_handshake_blocking()
{
    uint8_t tx_handshake[sizeof(iris_handshake_expected)];
    uint8_t rx_handshake[sizeof(iris_handshake_expected)] = {0};

    memcpy(tx_handshake, iris_handshake_expected, sizeof(tx_handshake));

    while (1)
    {
        HAL_StatusTypeDef ret;

        HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_RESET);
        printf("rx_handshake: %02x %02x %02x %02x\n", rx_handshake[0], rx_handshake[1], rx_handshake[2], rx_handshake[3]);
        my_sleep(100);
        ret = HAL_SPI_TransmitReceive(&hspi5,
                                      tx_handshake,
                                      rx_handshake,
                                      (uint16_t)sizeof(tx_handshake),
                                      ~0);
        my_sleep(100);
        HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_SET);
        printf("rx_handshake: %02x %02x %02x %02x\n", rx_handshake[0], rx_handshake[1], rx_handshake[2], rx_handshake[3]);

        if ((ret == HAL_OK) &&
            (memcmp(rx_handshake, iris_handshake_expected, sizeof(iris_handshake_expected)) == 0))
        {
            printf("IRIS SPI handshake complete\n");
            return;
        }

        printf("IRIS SPI handshake retry\n");
        HAL_Delay(500);
    }
}


void iris_transmit(const uint8_t *data, uint32_t size)
{
    iris_frame_nmbr++;
    uint32_t packet_nmbr;

    if ((data == NULL) || (size == 0U))
    {
        return;
    }

    packet_nmbr = (size + IRIS_PACKET_PAYLOAD_SIZE - 1U) / IRIS_PACKET_PAYLOAD_SIZE;
    if (packet_nmbr == 0U)
    {
        return;
    }

    for (uint32_t packet_idx = 0U; packet_idx < packet_nmbr; packet_idx++)
    {
        iris_packet_t pkt;
        uint32_t offset = packet_idx * IRIS_PACKET_PAYLOAD_SIZE;
        uint32_t remaining = size - offset;
        uint32_t chunk_len = (remaining > IRIS_PACKET_PAYLOAD_SIZE) ? IRIS_PACKET_PAYLOAD_SIZE : remaining;

        pkt.frame_nmbr = iris_frame_nmbr;
        pkt.packet_idx = packet_idx;
        pkt.packet_nmbr = packet_nmbr;

        memset(pkt.payload, 0, sizeof(pkt.payload));
        memcpy(pkt.payload, &data[offset], chunk_len);

        iris_transmit_raw((const uint8_t *)&pkt, (uint16_t)sizeof(pkt));
        my_sleep(100);

    }
}

// uint32_t packet_nmbr = DIV_ROUND_UP(rx_size, IRIS_PACKET_PAYLOAD_SIZE);
// 		for (uint32_t packet_idx = 0; packet_idx < packet_nmbr; packet_idx++) {
// 			iris_packet_t pkt = {
// 				.frame_nmbr = frame_nmbr,
// 				.packet_idx = packet_idx,
// 				.packet_nmbr = packet_nmbr,
// 			};

// 			size_t offset = packet_idx * IRIS_PACKET_PAYLOAD_SIZE;
// 			size_t chunk_len = MIN((size_t)IRIS_PACKET_PAYLOAD_SIZE,
// 					       (size_t)(rx_size - offset));

// 			memset(pkt.payload, 0, sizeof(pkt.payload));
// 			memcpy(pkt.payload, &spi_buffer[offset], chunk_len);

// 			/* Unique step color: UDP packet send */
// 			LED_TURN_RED();
// 			ret = zsock_sendto(ctx->sock_fd, &pkt, sizeof(pkt), 0,
// 					   &ctx->client_addr, ctx->client_addr_len);
// 			if (ret < 0) {
// 				LOG_ERR("UDP sendto failed frame=%u packet=%u (errno=%d)",
// 					frame_nmbr, packet_idx, errno);
// 				return COMM_FAILURE;
// 			}
// 		}

// 		LOG_INF("Frame %u: %u bytes sent in %u packet(s)",
// 			frame_nmbr, rx_size, packet_nmbr);