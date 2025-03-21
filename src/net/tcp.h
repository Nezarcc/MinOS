#ifndef TCP_H
#define TCP_H

#include "network.h"
#include "ip.h"
#include <stdint.h>

/* TCP flags */
#define TCP_FIN  0x01
#define TCP_SYN  0x02
#define TCP_RST  0x04
#define TCP_PSH  0x08
#define TCP_ACK  0x10
#define TCP_URG  0x20

/* TCP connection states */
#define TCP_STATE_CLOSED      0
#define TCP_STATE_LISTEN      1
#define TCP_STATE_SYN_SENT    2
#define TCP_STATE_SYN_RECEIVED 3
#define TCP_STATE_ESTABLISHED 4
#define TCP_STATE_FIN_WAIT_1  5
#define TCP_STATE_FIN_WAIT_2  6
#define TCP_STATE_CLOSE_WAIT  7
#define TCP_STATE_CLOSING     8
#define TCP_STATE_LAST_ACK    9
#define TCP_STATE_TIME_WAIT   10

/* TCP connection structure */
typedef struct tcp_connection {
    uint32_t local_ip;
    uint16_t local_port;
    uint32_t remote_ip;
    uint16_t remote_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t state;
    uint16_t window_size;
    struct tcp_connection* next;
} tcp_connection_t;

/* Initialize TCP protocol handler */
void tcp_init(void);

/* Process a TCP packet */
void tcp_process_packet(net_packet_t* packet, uint32_t offset);

/* Send a TCP packet */
int tcp_send_packet(tcp_connection_t* conn, uint8_t flags, void* data, uint32_t length);

/* Open a TCP connection */
tcp_connection_t* tcp_connect(uint32_t remote_ip, uint16_t remote_port);

/* Close a TCP connection */
void tcp_close(tcp_connection_t* conn);

/* Send data over a TCP connection */
int tcp_send(tcp_connection_t* conn, void* data, uint32_t length);

/* Receive data from a TCP connection */
int tcp_receive(tcp_connection_t* conn, void* buffer, uint32_t length);

/* Calculate TCP checksum */
uint16_t tcp_checksum(uint32_t src_ip, uint32_t dest_ip, tcp_header_t* header, void* data, uint32_t length);

#endif /* TCP_H */
