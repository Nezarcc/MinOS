#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>

/* Network interface structure */
typedef struct {
    char name[16];            /* Interface name (e.g., "eth0") */
    uint8_t mac_address[6];   /* MAC address */
    uint32_t ip_address;      /* IPv4 address */
    uint32_t netmask;         /* Network mask */
    uint32_t gateway;         /* Default gateway */
    uint8_t flags;            /* Interface flags */
    void* device_data;        /* Device-specific data */
} net_interface_t;

/* Interface flags */
#define NET_IF_UP       0x01  /* Interface is up */
#define NET_IF_RUNNING  0x02  /* Interface is running */
#define NET_IF_LOOPBACK 0x04  /* Interface is loopback */
#define NET_IF_DHCP     0x08  /* Interface uses DHCP */

/* Protocol types */
#define PROTO_IP   0x0800
#define PROTO_ARP  0x0806
#define PROTO_IPV6 0x86DD

/* Ethernet header */
typedef struct {
    uint8_t dest_mac[6];      /* Destination MAC address */
    uint8_t src_mac[6];       /* Source MAC address */
    uint16_t protocol;        /* Protocol type */
} __attribute__((packed)) eth_header_t;

/* IPv4 header */
typedef struct {
    uint8_t version_ihl;      /* Version and IHL */
    uint8_t tos;              /* Type of service */
    uint16_t total_length;    /* Total length */
    uint16_t id;              /* Identification */
    uint16_t flags_fragment;  /* Flags and fragment offset */
    uint8_t ttl;              /* Time to live */
    uint8_t protocol;         /* Protocol */
    uint16_t checksum;        /* Header checksum */
    uint32_t src_ip;          /* Source IP address */
    uint32_t dest_ip;         /* Destination IP address */
} __attribute__((packed)) ip_header_t;

/* TCP header */
typedef struct {
    uint16_t src_port;        /* Source port */
    uint16_t dest_port;       /* Destination port */
    uint32_t seq_num;         /* Sequence number */
    uint32_t ack_num;         /* Acknowledgment number */
    uint16_t flags;           /* Data offset, reserved, and flags */
    uint16_t window_size;     /* Window size */
    uint16_t checksum;        /* Checksum */
    uint16_t urgent_ptr;      /* Urgent pointer */
} __attribute__((packed)) tcp_header_t;

/* UDP header */
typedef struct {
    uint16_t src_port;        /* Source port */
    uint16_t dest_port;       /* Destination port */
    uint16_t length;          /* Length */
    uint16_t checksum;        /* Checksum */
} __attribute__((packed)) udp_header_t;

/* ARP header */
typedef struct {
    uint16_t hw_type;         /* Hardware type */
    uint16_t protocol;        /* Protocol type */
    uint8_t hw_addr_len;      /* Hardware address length */
    uint8_t proto_addr_len;   /* Protocol address length */
    uint16_t operation;       /* Operation */
    uint8_t sender_hw_addr[6];/* Sender hardware address */
    uint32_t sender_proto_addr;/* Sender protocol address */
    uint8_t target_hw_addr[6];/* Target hardware address */
    uint32_t target_proto_addr;/* Target protocol address */
} __attribute__((packed)) arp_header_t;

/* Network packet buffer */
typedef struct {
    uint8_t* data;            /* Packet data */
    uint32_t length;          /* Packet length */
    uint32_t capacity;        /* Buffer capacity */
    net_interface_t* interface;/* Interface the packet was received on */
} net_packet_t;

/* Initialize the network stack */
void network_init(void);

/* Register a network interface */
int network_register_interface(net_interface_t* interface);

/* Send a packet */
int network_send_packet(net_packet_t* packet);

/* Receive a packet */
net_packet_t* network_receive_packet(void);

/* Process a received packet */
void network_process_packet(net_packet_t* packet);

/* Allocate a packet buffer */
net_packet_t* network_alloc_packet(uint32_t size);

/* Free a packet buffer */
void network_free_packet(net_packet_t* packet);

/* Get an interface by name */
net_interface_t* network_get_interface(const char* name);

/* Convert an IP address string to a uint32_t */
uint32_t network_ip_to_uint(const char* ip);

/* Convert a uint32_t IP address to a string */
char* network_uint_to_ip(uint32_t ip, char* buffer);

/* Calculate IP checksum */
uint16_t network_checksum(void* data, uint32_t length);

#endif /* NETWORK_H */
