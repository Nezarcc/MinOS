#ifndef SOCKET_H
#define SOCKET_H

#include "network.h"
#include "tcp.h"
#include <stdint.h>

/* Socket types */
#define SOCK_STREAM     1  /* TCP */
#define SOCK_DGRAM      2  /* UDP */
#define SOCK_RAW        3  /* Raw IP */

/* Socket domains */
#define AF_INET         2  /* IPv4 */

/* Socket options */
#define SO_REUSEADDR    1
#define SO_KEEPALIVE    2
#define SO_BROADCAST    3
#define SO_LINGER       4

/* Socket states */
#define SOCKET_STATE_CLOSED      0
#define SOCKET_STATE_LISTENING   1
#define SOCKET_STATE_CONNECTING  2
#define SOCKET_STATE_CONNECTED   3

/* Socket structure */
typedef struct {
    uint8_t domain;          /* Socket domain (AF_INET) */
    uint8_t type;            /* Socket type (SOCK_STREAM, etc.) */
    uint8_t protocol;        /* Protocol (usually 0) */
    uint8_t state;           /* Socket state */
    uint32_t local_ip;       /* Local IP address */
    uint16_t local_port;     /* Local port */
    uint32_t remote_ip;      /* Remote IP address */
    uint16_t remote_port;    /* Remote port */
    void* protocol_data;     /* Protocol-specific data (e.g., TCP connection) */
} socket_t;

/* Initialize socket interface */
void socket_init(void);

/* Create a socket */
int socket_create(int domain, int type, int protocol);

/* Bind a socket to a local address */
int socket_bind(int sockfd, uint32_t addr, uint16_t port);

/* Connect a socket to a remote address */
int socket_connect(int sockfd, uint32_t addr, uint16_t port);

/* Listen for connections on a socket */
int socket_listen(int sockfd, int backlog);

/* Accept a connection on a socket */
int socket_accept(int sockfd, uint32_t* addr, uint16_t* port);

/* Send data on a socket */
int socket_send(int sockfd, const void* buf, uint32_t len, int flags);

/* Receive data from a socket */
int socket_recv(int sockfd, void* buf, uint32_t len, int flags);

/* Close a socket */
int socket_close(int sockfd);

/* Set socket options */
int socket_setsockopt(int sockfd, int level, int optname, const void* optval, uint32_t optlen);

/* Get socket options */
int socket_getsockopt(int sockfd, int level, int optname, void* optval, uint32_t* optlen);

#endif /* SOCKET_H */
