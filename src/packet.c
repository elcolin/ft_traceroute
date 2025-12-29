#include "packet.h"

inline void initPacket(void *buffer, t_udp_packet *packet)
{
    packet->ip_hdr = (void *) buffer;
    packet->udp_hdr = IPHDR_SHIFT(buffer);
}

inline uint16_t computeChecksum(uint8_t *addr, int count)
{
    uint32_t sum = 0;
    uint16_t *ptr = (uint16_t *) addr;

    while (count > 1) {
        sum += *ptr++;
        count -= 2;
    }
    if (count > 0) {
        sum += *((uint8_t *)ptr);
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (uint16_t)(~sum);
}

void defineRequestIPHeader(struct iphdr *ipHeader,
                           uint32_t src_ip,
                           uint32_t dst_ip,
                           uint8_t ttl,
                           uint16_t id)
{
    ipHeader->ihl = 5;                 // 5 * 4 = 20 bytes (no options)
    ipHeader->version = 4;
    ipHeader->tos = 0;
    ipHeader->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr));
    ipHeader->id = htons(id);
    ipHeader->frag_off = 0;
    ipHeader->ttl = ttl;
    ipHeader->protocol = IPPROTO_UDP;
    ipHeader->saddr = src_ip;
    ipHeader->daddr = dst_ip;
    ipHeader->check = 0;               // must be zero before computing
    ipHeader->check = computeChecksum((uint8_t *)ipHeader, sizeof(*ipHeader));
}

void defineRequestUDPHeader(struct iphdr *ipHeader, struct udphdr *udpHeader)
{
    // toodo change
    uint8_t tampon[1024] = {};
    static uint16_t dport = DEFAULT_DEST_PORT;
    udpHeader->uh_sport = htons(rand() | 0x8000);
    udpHeader->uh_dport = htons(dport++);
    udpHeader->uh_ulen = htons(sizeof(struct udphdr));
    t_pseudo_udp *psdudp = (t_pseudo_udp *) tampon;
    psdudp->daddr = ipHeader->daddr;
    psdudp->saddr = ipHeader->saddr;
    psdudp->zero = 0;
    psdudp->length = udpHeader->uh_ulen;
    psdudp->protocol = 17;
    memcpy(tampon + sizeof(t_pseudo_udp), udpHeader, sizeof(*udpHeader));
    udpHeader->uh_sum = computeChecksum(tampon, sizeof(*psdudp) + sizeof(*udpHeader));// udp checksum
}

inline status parsePacket(void *buffer, struct iphdr **ip_header, struct icmphdr **icmp_header)
{
    *ip_header = (struct iphdr *)buffer;   
    *icmp_header = (struct icmphdr *)(buffer + ((*ip_header)->ihl * 4));

    if (ntohs((*ip_header)->tot_len) < sizeof(struct iphdr))
        return FAILURE;
    return SUCCESS;
}