#ifndef SPI_CONFIG_H
#define SPI_CONFIG_H

#include "stm32n6xx_hal.h"

#define CS_PIN GPIO_PIN_3
#define CS_GPIO_PORT GPIOA

#define IRIS_PACKET_PAYLOAD_SIZE 1400U

typedef struct __attribute__((packed))
{
	uint32_t frame_nmbr;
	uint32_t packet_idx;
	uint32_t packet_nmbr;
    uint8_t payload[IRIS_PACKET_PAYLOAD_SIZE];
} iris_packet_t;

void iris_config();
void iris_handshake_blocking();
void iris_transmit(const uint8_t *data, uint32_t size);

#endif /* SPI_CONFIG_H */