#include "ip.h"
#include "network.h"
#include "../kernel/kernel.h"
#include <stdint.h>
#include <string.h>

/* IP packet ID counter */
static uint16_t ip_id_counter = 0;

/* Initialize IP protocol handler */
void ip_init() {
    terminal_writestring("Initializing IP protocol handler...\n");
    
    // Reset the ID counter
    ip_id_counter = 0;
    
    terminal_writestring("IP protocol handler initialized\n");
}

/* Process an IP packet */
void ip_process_packet(net_packet_t* packet, uint32_t offset) {
    if (!packet || !packet->data || offset + sizeof(ip_header_t) > packet->length) {
        return;
    }
    
    // Get the IP header
    ip_header_t* ip = (ip_header_t*)(packet->data + offset);
    
    // Verify the IP version
    uint8_t version = (ip->version_ihl >> 4) & 0x0F;
    if (version != 4) {
        // Only IPv4 is supported
        return;
    }
    
    // Calculate the header length
    uint8_t ihl = ip->version_ihl & 0x0F;
    uint32_t header_length = ihl * 4;
    
    // Verify the header length
    if (offset + header_length > packet->length) {
        return;
    }
    
    // Verify the checksum
    uint16_t original_checksum = ip->checksum;
    ip->checksum = 0;
    uint16_t calculated_checksum = network_checksum(ip, header_length);
    ip->checksum = original_checksum;
    
    if (original_checksum != calculated_checksum) {
        // Invalid checksum
        return;
    }
    
    // Convert the total length from network byte order
    uint16_t total_length = (ip->total_length >> 8) | ((ip->total_length & 0xFF) << 8);
    
    // Verify the total length
    if (offset + total_length > packet->length) {
        return;
    }
    
    // Check if the packet is for us
    uint32_t dest_ip = ip->dest_ip;
    bool for_us = false;
    
    // Check all interfaces
    for (uint32_t i = 0; i < MAX_INTERFACES; i++) {
        if (interfaces[i] && interfaces[i]->ip_address == dest_ip) {
            for_us = true;
            break;
        }
    }
    
    // Check for broadcast
    if (!for_us && dest_ip == 0xFFFFFFFF) {
        for_us = true;
    }
    
    if (!for_us) {
        // Not for us, ignore
        return;
    }
    
    // Process based on protocol
    switch (ip->protocol) {
        case IP_PROTO_ICMP:
            // Process ICMP packet
            // icmp_process_packet(packet, offset + header_length);
            break;
        case IP_PROTO_TCP:
            // Process TCP packet
            // tcp_process_packet(packet, offset + header_length);
            break;
        case IP_PROTO_UDP:
            // Process UDP packet
            // udp_process_packet(packet, offset + header_length);
            break;
        default:
            // Unknown protocol
            break;
    }
}

/* Send an IP packet */
int ip_send_packet(net_interface_t* interface, uint8_t protocol, uint32_t dest_ip, 
                  void* data, uint32_t length) {
    if (!interface || !data || length == 0) {
        return -1;
    }
    
    // Check if the interface is up
    if (!(interface->flags & NET_IF_UP)) {
        return -1;
    }
    
    // Calculate the total packet size
    uint32_t total_size = sizeof(eth_header_t) + sizeof(ip_header_t) + length;
    
    // Allocate a packet buffer
    net_packet_t* packet = network_alloc_packet(total_size);
    if (!packet) {
        return -1;
    }
    
    // Set the interface
    packet->interface = interface;
    
    // Set the packet length
    packet->length = total_size;
    
    // Get the Ethernet header
    eth_header_t* eth = (eth_header_t*)packet->data;
    
    // Set the Ethernet header fields
    // For now, we'll use a dummy destination MAC
    memset(eth->dest_mac, 0xFF, 6); // Broadcast
    memcpy(eth->src_mac, interface->mac_address, 6);
    eth->protocol = PROTO_IP;
    
    // Get the IP header
    ip_header_t* ip = (ip_header_t*)(packet->data + sizeof(eth_header_t));
    
    // Set the IP header fields
    ip->version_ihl = 0x45; // IPv4, 5 DWORDS (20 bytes)
    ip->tos = 0;
    ip->total_length = sizeof(ip_header_t) + length;
    ip->id = ip_id_counter++;
    ip->flags_fragment = 0;
    ip->ttl = 64;
    ip->protocol = protocol;
    ip->checksum = 0;
    ip->src_ip = interface->ip_address;
    ip->dest_ip = dest_ip;
    
    // Convert to network byte order
    ip->total_length = (ip->total_length >> 8) | ((ip->total_length & 0xFF) << 8);
    ip->id = (ip->id >> 8) | ((ip->id & 0xFF) << 8);
    
    // Calculate the checksum
    ip->checksum = network_checksum(ip, sizeof(ip_header_t));
    
    // Copy the data
    memcpy(packet->data + sizeof(eth_header_t) + sizeof(ip_header_t), data, length);
    
    // Send the packet
    int result = network_send_packet(packet);
    
    // Free the packet
    network_free_packet(packet);
    
    return result;
}

/* Calculate IP checksum */
uint16_t ip_checksum(ip_header_t* header) {
    if (!header) {
        return 0;
    }
    
    // Save the original checksum
    uint16_t original_checksum = header->checksum;
    
    // Set the checksum field to zero
    header->checksum = 0;
    
    // Calculate the checksum
    uint16_t checksum = network_checksum(header, sizeof(ip_header_t));
    
    // Restore the original checksum
    header->checksum = original_checksum;
    
    return checksum;
}
