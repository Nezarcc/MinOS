#include "tcp.h"
#include "network.h"
#include "ip.h"
#include "../kernel/kernel.h"
#include <stdint.h>
#include <string.h>

/* TCP connections list */
static tcp_connection_t* tcp_connections = NULL;

/* TCP port allocator */
static uint16_t next_ephemeral_port = 49152; // Start of dynamic/private ports

/* Initialize TCP protocol handler */
void tcp_init() {
    terminal_writestring("Initializing TCP protocol handler...\n");
    
    // Initialize the connections list
    tcp_connections = NULL;
    
    terminal_writestring("TCP protocol handler initialized\n");
}

/* Find a TCP connection */
static tcp_connection_t* tcp_find_connection(uint32_t local_ip, uint16_t local_port,
                                           uint32_t remote_ip, uint16_t remote_port) {
    tcp_connection_t* conn = tcp_connections;
    
    while (conn) {
        if (conn->local_ip == local_ip && conn->local_port == local_port &&
            conn->remote_ip == remote_ip && conn->remote_port == remote_port) {
            return conn;
        }
        conn = conn->next;
    }
    
    return NULL;
}

/* Create a new TCP connection */
static tcp_connection_t* tcp_create_connection(uint32_t local_ip, uint16_t local_port,
                                             uint32_t remote_ip, uint16_t remote_port) {
    // Allocate a new connection
    tcp_connection_t* conn = (tcp_connection_t*)kmalloc(sizeof(tcp_connection_t));
    if (!conn) {
        return NULL;
    }
    
    // Initialize the connection
    conn->local_ip = local_ip;
    conn->local_port = local_port;
    conn->remote_ip = remote_ip;
    conn->remote_port = remote_port;
    conn->seq_num = 0; // Should be randomized in a real implementation
    conn->ack_num = 0;
    conn->state = TCP_STATE_CLOSED;
    conn->window_size = 8192; // Default window size
    
    // Add to the connections list
    conn->next = tcp_connections;
    tcp_connections = conn;
    
    return conn;
}

/* Process a TCP packet */
void tcp_process_packet(net_packet_t* packet, uint32_t offset) {
    if (!packet || !packet->data || offset + sizeof(tcp_header_t) > packet->length) {
        return;
    }
    
    // Get the IP header
    ip_header_t* ip = (ip_header_t*)(packet->data + offset - sizeof(ip_header_t));
    
    // Get the TCP header
    tcp_header_t* tcp = (tcp_header_t*)(packet->data + offset);
    
    // Calculate the TCP header length
    uint8_t header_length = (tcp->flags >> 12) & 0x0F;
    header_length *= 4; // Convert to bytes
    
    // Verify the header length
    if (offset + header_length > packet->length) {
        return;
    }
    
    // Convert ports from network byte order
    uint16_t src_port = (tcp->src_port >> 8) | ((tcp->src_port & 0xFF) << 8);
    uint16_t dest_port = (tcp->dest_port >> 8) | ((tcp->dest_port & 0xFF) << 8);
    
    // Find the connection
    tcp_connection_t* conn = tcp_find_connection(ip->dest_ip, dest_port, ip->src_ip, src_port);
    
    // Extract TCP flags
    uint8_t flags = tcp->flags & 0x3F;
    
    // Handle based on connection state
    if (!conn) {
        // No existing connection
        
        // Check if it's a SYN packet (connection request)
        if (flags & TCP_SYN) {
            // Create a new connection
            conn = tcp_create_connection(ip->dest_ip, dest_port, ip->src_ip, src_port);
            if (!conn) {
                return;
            }
            
            // Set the connection state
            conn->state = TCP_STATE_SYN_RECEIVED;
            
            // Set the acknowledgment number
            conn->ack_num = (tcp->seq_num >> 24) | ((tcp->seq_num >> 8) & 0xFF00) | 
                           ((tcp->seq_num << 8) & 0xFF0000) | (tcp->seq_num << 24);
            conn->ack_num++; // Acknowledge the SYN
            
            // Send SYN+ACK
            tcp_send_packet(conn, TCP_SYN | TCP_ACK, NULL, 0);
        }
        // Ignore other packets for non-existent connections
        return;
    }
    
    // Handle based on connection state
    switch (conn->state) {
        case TCP_STATE_SYN_SENT:
            // We sent a SYN, waiting for SYN+ACK
            if ((flags & (TCP_SYN | TCP_ACK)) == (TCP_SYN | TCP_ACK)) {
                // Received SYN+ACK
                
                // Set the acknowledgment number
                conn->ack_num = (tcp->seq_num >> 24) | ((tcp->seq_num >> 8) & 0xFF00) | 
                               ((tcp->seq_num << 8) & 0xFF0000) | (tcp->seq_num << 24);
                conn->ack_num++; // Acknowledge the SYN
                
                // Update the sequence number
                conn->seq_num = (tcp->ack_num >> 24) | ((tcp->ack_num >> 8) & 0xFF00) | 
                               ((tcp->ack_num << 8) & 0xFF0000) | (tcp->ack_num << 24);
                
                // Set the connection state
                conn->state = TCP_STATE_ESTABLISHED;
                
                // Send ACK
                tcp_send_packet(conn, TCP_ACK, NULL, 0);
            }
            break;
            
        case TCP_STATE_SYN_RECEIVED:
            // We received a SYN and sent SYN+ACK, waiting for ACK
            if (flags & TCP_ACK) {
                // Received ACK
                
                // Update the sequence number
                conn->seq_num = (tcp->ack_num >> 24) | ((tcp->ack_num >> 8) & 0xFF00) | 
                               ((tcp->ack_num << 8) & 0xFF0000) | (tcp->ack_num << 24);
                
                // Set the connection state
                conn->state = TCP_STATE_ESTABLISHED;
            }
            break;
            
        case TCP_STATE_ESTABLISHED:
            // Connection established, handle data
            
            // Check for data
            uint32_t data_offset = offset + header_length;
            uint32_t data_length = packet->length - data_offset;
            
            if (data_length > 0) {
                // Process the data
                // In a real implementation, this would pass the data to the socket layer
                
                // Update the acknowledgment number
                uint32_t seq_num = (tcp->seq_num >> 24) | ((tcp->seq_num >> 8) & 0xFF00) | 
                                  ((tcp->seq_num << 8) & 0xFF0000) | (tcp->seq_num << 24);
                conn->ack_num = seq_num + data_length;
                
                // Send ACK
                tcp_send_packet(conn, TCP_ACK, NULL, 0);
            }
            
            // Check for FIN (connection close request)
            if (flags & TCP_FIN) {
                // Update the acknowledgment number
                conn->ack_num++;
                
                // Set the connection state
                conn->state = TCP_STATE_CLOSE_WAIT;
                
                // Send ACK
                tcp_send_packet(conn, TCP_ACK, NULL, 0);
                
                // Send FIN+ACK
                tcp_send_packet(conn, TCP_FIN | TCP_ACK, NULL, 0);
                
                // Set the connection state
                conn->state = TCP_STATE_LAST_ACK;
            }
            break;
            
        case TCP_STATE_FIN_WAIT_1:
            // We sent a FIN, waiting for ACK
            if (flags & TCP_ACK) {
                // Received ACK
                
                // Set the connection state
                conn->state = TCP_STATE_FIN_WAIT_2;
            }
            
            // Check for FIN (simultaneous close)
            if (flags & TCP_FIN) {
                // Update the acknowledgment number
                conn->ack_num++;
                
                // Send ACK
                tcp_send_packet(conn, TCP_ACK, NULL, 0);
                
                // Set the connection state
                if (conn->state == TCP_STATE_FIN_WAIT_2) {
                    conn->state = TCP_STATE_TIME_WAIT;
                } else {
                    conn->state = TCP_STATE_CLOSING;
                }
            }
            break;
            
        case TCP_STATE_FIN_WAIT_2:
            // We sent a FIN and received ACK, waiting for FIN
            if (flags & TCP_FIN) {
                // Received FIN
                
                // Update the acknowledgment number
                conn->ack_num++;
                
                // Send ACK
                tcp_send_packet(conn, TCP_ACK, NULL, 0);
                
                // Set the connection state
                conn->state = TCP_STATE_TIME_WAIT;
            }
            break;
            
        case TCP_STATE_CLOSING:
            // Both sides sent FIN, we're waiting for ACK
            if (flags & TCP_ACK) {
                // Received ACK
                
                // Set the connection state
                conn->state = TCP_STATE_TIME_WAIT;
            }
            break;
            
        case TCP_STATE_LAST_ACK:
            // We received a FIN and sent FIN+ACK, waiting for ACK
            if (flags & TCP_ACK) {
                // Received ACK
                
                // Set the connection state
                conn->state = TCP_STATE_CLOSED;
                
                // Remove the connection
                // In a real implementation, this would free the connection
            }
            break;
            
        case TCP_STATE_TIME_WAIT:
            // Wait for a while before closing
            // In a real implementation, this would set a timer
            break;
            
        default:
            // Ignore packets in other states
            break;
    }
}

/* Send a TCP packet */
int tcp_send_packet(tcp_connection_t* conn, uint8_t flags, void* data, uint32_t length) {
    if (!conn) {
        return -1;
    }
    
    // Calculate the total packet size
    uint32_t total_size = sizeof(tcp_header_t) + length;
    
    // Allocate a buffer for the TCP packet
    uint8_t* buffer = (uint8_t*)kmalloc(total_size);
    if (!buffer) {
        return -1;
    }
    
    // Get the TCP header
    tcp_header_t* tcp = (tcp_header_t*)buffer;
    
    // Set the TCP header fields
    tcp->src_port = conn->local_port;
    tcp->dest_port = conn->remote_port;
    tcp->seq_num = conn->seq_num;
    tcp->ack_num = conn->ack_num;
    tcp->flags = (5 << 12) | flags; // 5 DWORDS (20 bytes) header
    tcp->window_size = conn->window_size;
    tcp->checksum = 0;
    tcp->urgent_ptr = 0;
    
    // Convert to network byte order
    tcp->src_port = (tcp->src_port >> 8) | ((tcp->src_port & 0xFF) << 8);
    tcp->dest_port = (tcp->dest_port >> 8) | ((tcp->dest_port & 0xFF) << 8);
    tcp->seq_num = (tcp->seq_num >> 24) | ((tcp->seq_num >> 8) & 0xFF00) | 
                  ((tcp->seq_num << 8) & 0xFF0000) | (tcp->seq_num << 24);
    tcp->ack_num = (tcp->ack_num >> 24) | ((tcp->ack_num >> 8) & 0xFF00) | 
                  ((tcp->ack_num << 8) & 0xFF0000) | (tcp->ack_num << 24);
    tcp->window_size = (tcp->window_size >> 8) | ((tcp->window_size & 0xFF) << 8);
    
    // Copy the data
    if (data && length > 0) {
        memcpy(buffer + sizeof(tcp_header_t), data, length);
    }
    
    // Calculate the checksum
    tcp->checksum = tcp_checksum(conn->local_ip, conn->remote_ip, tcp, data, length);
    
    // Send the IP packet
    int result = ip_send_packet(network_get_interface("eth0"), IP_PROTO_TCP, conn->remote_ip, buffer, total_size);
    
    // Free the buffer
    kfree(buffer);
    
    // Update the sequence number if we sent data or SYN/FIN
    if (length > 0) {
        conn->seq_num += length;
    }
    if (flags & (TCP_SYN | TCP_FIN)) {
        conn->seq_num++;
    }
    
    return result;
}

/* Open a TCP connection */
tcp_connection_t* tcp_connect(uint32_t remote_ip, uint16_t remote_port) {
    // Find a suitable interface
    net_interface_t* interface = network_get_interface("eth0");
    if (!interface) {
        return NULL;
    }
    
    // Allocate a local port
    uint16_t local_port = next_ephemeral_port++;
    if (next_ephemeral_port > 65535) {
        next_ephemeral_port = 49152;
    }
    
    // Create a new connection
    tcp_connection_t* conn = tcp_create_connection(interface->ip_address, local_port, remote_ip, remote_port);
    if (!conn) {
        return NULL;
    }
    
    // Set the initial sequence number
    conn->seq_num = 0; // Should be randomized in a real implementation
    
    // Set the connection state
    conn->state = TCP_STATE_SYN_SENT;
    
    // Send SYN
    tcp_send_packet(conn, TCP_SYN, NULL, 0);
    
    return conn;
}

/* Close a TCP connection */
void tcp_close(tcp_connection_t* conn) {
    if (!conn) {
        return;
    }
    
    // Check the connection state
    switch (conn->state) {
        case TCP_STATE_CLOSED:
        case TCP_STATE_LISTEN:
            // Already closed or no connection established
            break;
            
        case TCP_STATE_SYN_SENT:
            // Connection not established yet
            conn->state = TCP_STATE_CLOSED;
            break;
            
        case TCP_STATE_SYN_RECEIVED:
        case TCP_STATE_ESTABLISHED:
            // Send FIN
            tcp_send_packet(conn, TCP_FIN, NULL, 0);
            
            // Set the connection state
            conn->state = TCP_STATE_FIN_WAIT_1;
            break;
            
        case TCP_STATE_CLOSE_WAIT:
            // Send FIN
            tcp_send_packet(conn, TCP_FIN, NULL, 0);
            
            // Set the connection state
            conn->state = TCP_STATE_LAST_ACK;
            break;
            
        default:
            // Ignore in other states
            break;
    }
}

/* Send data over a TCP connection */
int tcp_send(tcp_connection_t* conn, void* data, uint32_t length) {
    if (!conn || !data || length == 0) {
        return -1;
    }
    
    // Check if the connection is established
    if (conn->state != TCP_STATE_ESTABLISHED) {
        return -1;
    }
    
    // Send the data
    return tcp_send_packet(conn, TCP_PSH | TCP_ACK, data, length);
}

/* Receive data from a TCP connection */
int tcp_receive(tcp_connection_t* conn, void* buffer, uint32_t length) {
    // This would be implemented in a real TCP stack
    // For now, we'll just return 0 (no data available)
    return 0;
}

/* Calculate TCP checksum */
uint16_t tcp_checksum(uint32_t src_ip, uint32_t dest_ip, tcp_header_t* header, void* data, uint32_t length) {
    // TCP checksum includes a pseudo-header
    uint32_t sum = 0;
    
    // Add source IP
    sum += (src_ip >> 16) & 0xFFFF;
    sum += src_ip & 0xFFFF;
    
    // Add destination IP
    sum += (dest_ip >> 16) & 0xFFFF;
    sum += dest_ip & 0xFFFF;
    
    // Add protocol and TCP length
    sum += IP_PROTO_TCP;
    sum += sizeof(tcp_header_t) + length;
    
    // Add the TCP header
    uint16_t* ptr = (uint16_t*)header;
    for (uint32_t i = 0; i < sizeof(tcp_header_t) / 2; i++) {
        sum += *ptr++;
    }
    
    // Add the data
    ptr = (uint16_t*)data;
    uint32_t data_words = length / 2;
    for (uint32_t i = 0; i < data_words; i++) {
        sum += *ptr++;
    }
    
    // Add the remaining byte if length is odd
    if (length % 2) {
        sum += *(uint8_t*)ptr;
    }
    
    // Add the carry
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    // Take the one's complement
    return ~sum;
}
