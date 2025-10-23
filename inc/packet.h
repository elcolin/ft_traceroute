#ifndef PACKET_H
#define PACKET_H
// ip header
#include <netinet/ip.h>
// icmp header 
#include <netinet/ip_icmp.h>
#include <stddef.h>
//memset
#include <string.h>
//getpid
#include <unistd.h>

#include "utils.h"
// #include "args.h"

#define ICMP_CODE 0
#define BUFFER_SIZE 65535 //-> max buffer size 65 535
#define DEFAULT_PADDING 56

#define IPHDR_SIZE (sizeof(struct iphdr))
#define ICMPHDR_SIZE (sizeof(struct icmphdr))

#define IPHDR_SHIFT(start) ((void *)start + IPHDR_SIZE)
#define ICMPHDR_SHIFT(start) ((void *)start + ICMPHDR_SIZE)

typedef struct s_packet {
    uint8_t         buffer[BUFFER_SIZE];
    struct iphdr    *ip_hdr;
    struct icmphdr  *icmp_hdr;
}   t_packet;

void        initPacket(t_packet *packet);
uint16_t    computeChecksum(uint8_t *addr, int count);
void        defineRequestPacket(t_packet *request,
                        uint32_t src_ip,
                        uint32_t dst_ip,
                        uint16_t sequenceNumber);
void        defineRequestIPHeader(struct iphdr *ipHeader,
                           uint32_t src_ip,
                           uint32_t dst_ip,
                           uint16_t sequenceNumber);
void        defineRequestICMPHeader(struct icmphdr *icmpHeader, u_int16_t sequenceNumber);
status      comparePackets(struct icmphdr *icmp_reply, struct icmphdr *icmp_request);
int         parsePacket(void *buffer, struct iphdr **ip_header, struct icmphdr **icmp_header);
status      getValidPacket(t_packet *reply, t_packet *request);


#endif