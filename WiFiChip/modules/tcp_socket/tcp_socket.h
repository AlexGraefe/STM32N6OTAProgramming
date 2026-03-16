#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#define SERVER_IP        "192.168.5.29"
#define SERVER_PORT      8081
#define BUFFER_SIZE      256

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>

#define TCP_DATA_MAX_SIZE 60000

#define SOCKET_THREAD_PRIORITY 100

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

#define SPI_NODE DT_ALIAS(spi)

#define IRIS_PACKET_PAYLOAD_SIZE 1024U
#define SPI_MAX_FRAME_SIZE       60000U

typedef struct __attribute__((packed))
{
	uint32_t packet_idx;
	uint32_t packet_nmbr;
    uint8_t payload[IRIS_PACKET_PAYLOAD_SIZE];
} iris_packet_t;

typedef enum {
	COMM_WIFI_CONNECTING,
	COMM_WAITING_FOR_IP,
	COMM_ESTABLISHING_SERVER,
	COMM_RECEIVING_MESSAGES,
	COMM_FAILURE,
	COMM_CLEANUP,
	COMM_DONE,
} communication_state_t;

typedef struct {
	struct sockaddr_in server_addr;
	char               buffer[BUFFER_SIZE];
	char               ip_addr[NET_IPV4_ADDR_LEN];
	int  sock_fd;
	bool wifi_connected;
	bool socket_open;
	int  exit_code;
	communication_state_t failure_from_state;
} communication_context_t;

typedef struct {
	uint8_t enabled;
} socket_start_message_t;

typedef struct {
	uint32_t packet_idx;
	int32_t status;
} socket_spi_ack_message_t;

int run_tcp_socket_demo(void);
struct k_msgq *get_socket_packet_msgq(void);
struct k_msgq *get_socket_start_msgq(void);
struct k_msgq *get_socket_spi_ack_msgq(void);

#endif /* TCP_SOCKET_H */
