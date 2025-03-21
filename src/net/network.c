#include "network.h"
#include "../kernel/kernel.h"
#include <stdint.h>
#include <string.h>

/* Maximum number of network interfaces */
#define MAX_INTERFACES 8

/* Network interfaces */
static net_interface_t* interfaces[MAX_INTERFACES];
static uint32_t num_interfaces = 0;

/* Initialize the network stack */
void network_init() {
    terminal_writestring("Initializing network stack...\n");
    
    // Initialize interfaces array
    for (int i = 0; i < MAX_INTERFACES; i++) {
        interfaces[i] = NULL;
    }
    
    // Create loopback interface
    net_interface_t* loopback = (net_interface_t*)kmalloc(sizeof(net_interface_t));
    memset(loopback, 0, sizeof(net_interface_t));
    
    strcpy(loopback->name, "lo");
    // MAC address for loopback is all zeros
    loopback->ip_address = network_ip_to_uint("127.0.0.1");
    loopback->netmask = network_ip_to_uint("255.0.0.0");
    loopback->gateway = 0;
    loopback->flags = NET_IF_UP | NET_IF_RUNNING | NET_IF_LOOPBACK;
    
    // Register loopback interface
    network_register_interface(loopback);
    
    terminal_writestring("Network stack initialized\n");
}

/* Register a network interface */
int network_register_interface(net_interface_t* interface) {
    if (!interface) {
        return -1;
    }
    
    // Check if we have room for another interface
    if (num_interfaces >= MAX_INTERFACES) {
        return -1;
    }
    
    // Add the interface to the array
    interfaces[num_interfaces++] = interface;
    
    // Log the interface registration
    terminal_writestring("Registered network interface: ");
    terminal_writestring(interface->name);
    terminal_writestring("\n");
    
    return 0;
}

/* Send a packet */
int network_send_packet(net_packet_t* packet) {
    if (!packet || !packet->data || !packet->interface) {
        return -1;
    }
    
    // Check if the interface is up
    if (!(packet->interface->flags & NET_IF_UP)) {
        return -1;
    }
    
    // In a real implementation, this would call the device driver's send function
    // For now, we'll just log the packet
    terminal_writestring("Sending packet on interface ");
    terminal_writestring(packet->interface->name);
    terminal_writestring(", length: ");
    // Print packet length
    terminal_writestring("\n");
    
    return 0;
}

/* Receive a packet */
net_packet_t* network_receive_packet() {
    // In a real implementation, this would check for received packets
    // For now, we'll just return NULL
    return NULL;
}

/* Process a received packet */
void network_process_packet(net_packet_t* packet) {
    if (!packet || !packet->data) {
        return;
    }
    
    // Get the Ethernet header
    eth_header_t* eth = (eth_header_t*)packet->data;
    
    // Check the protocol type
    switch (eth->protocol) {
        case PROTO_IP:
            // Process IP packet
            // To be implemented
            break;
        case PROTO_ARP:
            // Process ARP packet
            // To be implemented
            break;
        default:
            // Unknown protocol
            break;
    }
}

/* Allocate a packet buffer */
net_packet_t* network_alloc_packet(uint32_t size) {
    // Allocate the packet structure
    net_packet_t* packet = (net_packet_t*)kmalloc(sizeof(net_packet_t));
    if (!packet) {
        return NULL;
    }
    
    // Allocate the data buffer
    packet->data = (uint8_t*)kmalloc(size);
    if (!packet->data) {
        kfree(packet);
        return NULL;
    }
    
    // Initialize the packet
    packet->length = 0;
    packet->capacity = size;
    packet->interface = NULL;
    
    return packet;
}

/* Free a packet buffer */
void network_free_packet(net_packet_t* packet) {
    if (!packet) {
        return;
    }
    
    // Free the data buffer
    if (packet->data) {
        kfree(packet->data);
    }
    
    // Free the packet structure
    kfree(packet);
}

/* Get an interface by name */
net_interface_t* network_get_interface(const char* name) {
    if (!name) {
        return NULL;
    }
    
    // Search for the interface
    for (uint32_t i = 0; i < num_interfaces; i++) {
        if (strcmp(interfaces[i]->name, name) == 0) {
            return interfaces[i];
        }
    }
    
    return NULL;
}

/* Convert an IP address string to a uint32_t */
uint32_t network_ip_to_uint(const char* ip) {
    if (!ip) {
        return 0;
    }
    
    uint32_t result = 0;
    uint8_t octet = 0;
    uint8_t i = 0;
    
    while (*ip) {
        if (*ip == '.') {
            result = (result << 8) | octet;
            octet = 0;
            i++;
        } else if (*ip >= '0' && *ip <= '9') {
            octet = octet * 10 + (*ip - '0');
        }
        ip++;
    }
    
    // Add the last octet
    result = (result << 8) | octet;
    
    return result;
}

/* Convert a uint32_t IP address to a string */
char* network_uint_to_ip(uint32_t ip, char* buffer) {
    if (!buffer) {
        return NULL;
    }
    
    uint8_t a = (ip >> 24) & 0xFF;
    uint8_t b = (ip >> 16) & 0xFF;
    uint8_t c = (ip >> 8) & 0xFF;
    uint8_t d = ip & 0xFF;
    
    // Format the IP address string
    // In a real implementation, this would use sprintf
    buffer[0] = '0' + a / 100;
    buffer[1] = '0' + (a / 10) % 10;
    buffer[2] = '0' + a % 10;
    buffer[3] = '.';
    buffer[4] = '0' + b / 100;
    buffer[5] = '0' + (b / 10) % 10;
    buffer[6] = '0' + b % 10;
    buffer[7] = '.';
    buffer[8] = '0' + c / 100;
    buffer[9] = '0' + (c / 10) % 10;
    buffer[10] = '0' + c % 10;
    buffer[11] = '.';
    buffer[12] = '0' + d / 100;
    buffer[13] = '0' + (d / 10) % 10;
    buffer[14] = '0' + d % 10;
    buffer[15] = '\0';
    
    return buffer;
}

/* Calculate IP checksum */
uint16_t network_checksum(void* data, uint32_t length) {
    uint32_t sum = 0;
    uint16_t* ptr = (uint16_t*)data;
    
    // Sum all 16-bit words
    while (length > 1) {
        sum += *ptr++;
        length -= 2;
    }
    
    // Add the remaining byte if length is odd
    if (length > 0) {
        sum += *(uint8_t*)ptr;
    }
    
    // Add the carry
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    // Take the one's complement
    return ~sum;
}
