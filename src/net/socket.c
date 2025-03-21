#include "socket.h"
#include "network.h"
#include "tcp.h"
#include "../kernel/kernel.h"
#include <stdint.h>
#include <string.h>

/* Maximum number of sockets */
#define MAX_SOCKETS 128

/* Socket table */
static socket_t* socket_table[MAX_SOCKETS];

/* Initialize socket interface */
void socket_init() {
    terminal_writestring("Initializing socket interface...\n");
    
    // Initialize socket table
    for (int i = 0; i < MAX_SOCKETS; i++) {
        socket_table[i] = NULL;
    }
    
    terminal_writestring("Socket interface initialized\n");
}

/* Allocate a socket descriptor */
static int alloc_socket_descriptor() {
    for (int i = 0; i < MAX_SOCKETS; i++) {
        if (socket_table[i] == NULL) {
            return i;
        }
    }
    return -1; // No free socket descriptors
}

/* Create a socket */
int socket_create(int domain, int type, int protocol) {
    // Check parameters
    if (domain != AF_INET) {
        return -1; // Only AF_INET is supported
    }
    
    if (type != SOCK_STREAM && type != SOCK_DGRAM && type != SOCK_RAW) {
        return -1; // Unsupported socket type
    }
    
    // Allocate a socket descriptor
    int sockfd = alloc_socket_descriptor();
    if (sockfd < 0) {
        return -1; // No free socket descriptors
    }
    
    // Allocate a socket structure
    socket_t* socket = (socket_t*)kmalloc(sizeof(socket_t));
    if (!socket) {
        return -1; // Out of memory
    }
    
    // Initialize the socket
    memset(socket, 0, sizeof(socket_t));
    socket->domain = domain;
    socket->type = type;
    socket->protocol = protocol;
    socket->state = SOCKET_STATE_CLOSED;
    
    // Store the socket
    socket_table[sockfd] = socket;
    
    return sockfd;
}

/* Bind a socket to a local address */
int socket_bind(int sockfd, uint32_t addr, uint16_t port) {
    // Check if the socket descriptor is valid
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || socket_table[sockfd] == NULL) {
        return -1;
    }
    
    // Get the socket
    socket_t* socket = socket_table[sockfd];
    
    // Check if the socket is already bound
    if (socket->local_port != 0) {
        return -1; // Already bound
    }
    
    // Set the local address and port
    socket->local_ip = addr;
    socket->local_port = port;
    
    return 0;
}

/* Connect a socket to a remote address */
int socket_connect(int sockfd, uint32_t addr, uint16_t port) {
    // Check if the socket descriptor is valid
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || socket_table[sockfd] == NULL) {
        return -1;
    }
    
    // Get the socket
    socket_t* socket = socket_table[sockfd];
    
    // Check if the socket is of the right type
    if (socket->type != SOCK_STREAM) {
        return -1; // Only SOCK_STREAM supports connect
    }
    
    // Check if the socket is already connected
    if (socket->state == SOCKET_STATE_CONNECTED) {
        return -1; // Already connected
    }
    
    // If the socket is not bound, bind it to a random local port
    if (socket->local_port == 0) {
        // Find a suitable interface
        net_interface_t* interface = network_get_interface("eth0");
        if (!interface) {
            return -1;
        }
        
        socket->local_ip = interface->ip_address;
        socket->local_port = 49152 + (sockfd % 16384); // Simple port allocation
    }
    
    // Set the remote address and port
    socket->remote_ip = addr;
    socket->remote_port = port;
    
    // Set the socket state
    socket->state = SOCKET_STATE_CONNECTING;
    
    // Create a TCP connection
    tcp_connection_t* conn = tcp_connect(addr, port);
    if (!conn) {
        socket->state = SOCKET_STATE_CLOSED;
        return -1;
    }
    
    // Store the TCP connection
    socket->protocol_data = conn;
    
    // Set the socket state
    socket->state = SOCKET_STATE_CONNECTED;
    
    return 0;
}

/* Listen for connections on a socket */
int socket_listen(int sockfd, int backlog) {
    // Check if the socket descriptor is valid
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || socket_table[sockfd] == NULL) {
        return -1;
    }
    
    // Get the socket
    socket_t* socket = socket_table[sockfd];
    
    // Check if the socket is of the right type
    if (socket->type != SOCK_STREAM) {
        return -1; // Only SOCK_STREAM supports listen
    }
    
    // Check if the socket is bound
    if (socket->local_port == 0) {
        return -1; // Socket not bound
    }
    
    // Set the socket state
    socket->state = SOCKET_STATE_LISTENING;
    
    // In a real implementation, this would set up a listening TCP socket
    
    return 0;
}

/* Accept a connection on a socket */
int socket_accept(int sockfd, uint32_t* addr, uint16_t* port) {
    // Check if the socket descriptor is valid
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || socket_table[sockfd] == NULL) {
        return -1;
    }
    
    // Get the socket
    socket_t* socket = socket_table[sockfd];
    
    // Check if the socket is of the right type
    if (socket->type != SOCK_STREAM) {
        return -1; // Only SOCK_STREAM supports accept
    }
    
    // Check if the socket is listening
    if (socket->state != SOCKET_STATE_LISTENING) {
        return -1; // Socket not listening
    }
    
    // In a real implementation, this would wait for an incoming connection
    // For now, we'll just return an error
    return -1;
}

/* Send data on a socket */
int socket_send(int sockfd, const void* buf, uint32_t len, int flags) {
    // Check if the socket descriptor is valid
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || socket_table[sockfd] == NULL) {
        return -1;
    }
    
    // Get the socket
    socket_t* socket = socket_table[sockfd];
    
    // Check if the socket is connected
    if (socket->state != SOCKET_STATE_CONNECTED) {
        return -1; // Socket not connected
    }
    
    // Check the socket type
    if (socket->type == SOCK_STREAM) {
        // TCP socket
        tcp_connection_t* conn = (tcp_connection_t*)socket->protocol_data;
        if (!conn) {
            return -1;
        }
        
        return tcp_send(conn, (void*)buf, len);
    } else if (socket->type == SOCK_DGRAM) {
        // UDP socket
        // To be implemented
        return -1;
    } else {
        // Unsupported socket type
        return -1;
    }
}

/* Receive data from a socket */
int socket_recv(int sockfd, void* buf, uint32_t len, int flags) {
    // Check if the socket descriptor is valid
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || socket_table[sockfd] == NULL) {
        return -1;
    }
    
    // Get the socket
    socket_t* socket = socket_table[sockfd];
    
    // Check if the socket is connected
    if (socket->state != SOCKET_STATE_CONNECTED) {
        return -1; // Socket not connected
    }
    
    // Check the socket type
    if (socket->type == SOCK_STREAM) {
        // TCP socket
        tcp_connection_t* conn = (tcp_connection_t*)socket->protocol_data;
        if (!conn) {
            return -1;
        }
        
        return tcp_receive(conn, buf, len);
    } else if (socket->type == SOCK_DGRAM) {
        // UDP socket
        // To be implemented
        return -1;
    } else {
        // Unsupported socket type
        return -1;
    }
}

/* Close a socket */
int socket_close(int sockfd) {
    // Check if the socket descriptor is valid
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || socket_table[sockfd] == NULL) {
        return -1;
    }
    
    // Get the socket
    socket_t* socket = socket_table[sockfd];
    
    // Close the socket based on its type
    if (socket->type == SOCK_STREAM && socket->protocol_data) {
        // TCP socket
        tcp_close((tcp_connection_t*)socket->protocol_data);
    }
    
    // Free the socket
    kfree(socket);
    
    // Clear the socket table entry
    socket_table[sockfd] = NULL;
    
    return 0;
}

/* Set socket options */
int socket_setsockopt(int sockfd, int level, int optname, const void* optval, uint32_t optlen) {
    // Check if the socket descriptor is valid
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || socket_table[sockfd] == NULL) {
        return -1;
    }
    
    // Get the socket
    socket_t* socket = socket_table[sockfd];
    
    // Handle socket options
    // In a real implementation, this would set various socket options
    
    return 0;
}

/* Get socket options */
int socket_getsockopt(int sockfd, int level, int optname, void* optval, uint32_t* optlen) {
    // Check if the socket descriptor is valid
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || socket_table[sockfd] == NULL) {
        return -1;
    }
    
    // Get the socket
    socket_t* socket = socket_table[sockfd];
    
    // Handle socket options
    // In a real implementation, this would get various socket options
    
    return 0;
}
