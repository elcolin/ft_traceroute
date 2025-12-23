#ifndef PACKET_H
#define PACKET_H
// ip header
#include <netinet/ip.h>
// icmp header 
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <stddef.h>
//memset
#include <string.h>
//getpid
#include <unistd.h>
//rand
#include <stdlib.h>

#include "utils.h"
// #include "args.h"

//TO DO Remove
#include <stdio.h>


#define ICMP_CODE 0
#define BUFFER_SIZE 65535 //-> max buffer size 65 535
#define DEFAULT_PADDING 56
#define DEFAULT_DEST_PORT 33434

#define IPHDR_SIZE (sizeof(struct iphdr))
#define ICMPHDR_SIZE (sizeof(struct icmphdr))

#define IPHDR_SHIFT(start) ((void *)start + IPHDR_SIZE)
#define ICMPHDR_SHIFT(start) ((void *)start + ICMPHDR_SIZE)

typedef struct s_pseudo_udp {
    uint32_t saddr;	/* source addr */
    uint32_t daddr;	/* destination addr */
    uint8_t  zero;
    uint8_t  protocol;		
    uint16_t length; /* udp length */
} t_pseudo_udp;

typedef struct s_packet {
    struct iphdr    *ip_hdr;
    struct udphdr   *udp_hdr;
    struct icmphdr  *icmp_hdr;
}   t_packet;

void        initPacket(void *buffer, t_packet *packet);
uint16_t    computeChecksum(uint8_t *addr, int count);
void defineRequestIPHeader(struct iphdr *ipHeader,
                           uint32_t src_ip,
                           uint32_t dst_ip,
                           uint8_t ttl,
                           uint16_t id);
void defineRequestICMPHeader(struct icmphdr *icmpHeader, uint16_t id, u_int16_t sequenceNumber);
void defineRequestUDPHeader(struct iphdr *ipHeader, struct udphdr *udpHeader);
status parsePacket(void *buffer, struct iphdr **ip_header, struct icmphdr **icmp_header);

#endif