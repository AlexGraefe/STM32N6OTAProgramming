#ifndef SPI_CONFIG_H
#define SPI_CONFIG_H

#include "stm32n6xx_hal.h"

#define CS_PIN GPIO_PIN_3
#define CS_GPIO_PORT GPIOA

#define IRIS_PACKET_PAYLOAD_SIZE 1024U

typedef struct __attribute__((packed))
{
	uint32_t packet_idx;
	uint32_t packet_nmbr;
    uint8_t payload[IRIS_PACKET_PAYLOAD_SIZE];
} iris_packet_t;

void iris_config();
void iris_handshake_blocking();
int iris_receive_packet_blocking(iris_packet_t *packet);

#endif /* SPI_CONFIG_H */