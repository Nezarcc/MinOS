#ifndef IP_H
#define IP_H

#include "network.h"
#include <stdint.h>

/* IP protocols */
#define IP_PROTO_ICMP 1
#define IP_PROTO_TCP  6
#define IP_PROTO_UDP  17

/* Initialize IP protocol handler */
void ip_init(void);

/* Process an IP packet */
void ip_process_packet(net_packet_t* packet, uint32_t offset);

/* Send an IP packet */
int ip_send_packet(net_interface_t* interface, uint8_t protocol, uint32_t dest_ip, 
                  void* data, uint32_t length);

/* Calculate IP checksum */
uint16_t ip_checksum(ip_header_t* header);

#endif /* IP_H */
