#include "packet.h"

void initPacket(void *buffer, t_packet *packet)
{
    packet->ip_hdr = (void *) buffer;
    packet->icmp_hdr = IPHDR_SHIFT(buffer);
}

uint16_t computeChecksum(uint8_t *addr, int count)
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
    ipHeader->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr) + DEFAULT_PADDING);
    ipHeader->id = htons(id);
    ipHeader->frag_off = 0;
    ipHeader->ttl = ttl;
    ipHeader->protocol = IPPROTO_ICMP;
    ipHeader->saddr = src_ip;
    ipHeader->daddr = dst_ip;
    ipHeader->check = 0;               // must be zero before computing
    ipHeader->check = computeChecksum((uint8_t *)ipHeader, sizeof(*ipHeader));
}

void defineRequestICMPHeader(struct icmphdr *icmpHeader, uint16_t id, u_int16_t sequenceNumber)
{
    // Setting up the ICMP header
    icmpHeader->type = ICMP_ECHO;
    icmpHeader->code = ICMP_CODE;
    icmpHeader->un.echo.id = htons(id);
    icmpHeader->un.echo.sequence = htons(sequenceNumber);
    icmpHeader->checksum = 0;
    icmpHeader->checksum = computeChecksum((uint8_t *)icmpHeader, sizeof(*icmpHeader));
}

status parsePacket(void *buffer, struct iphdr **ip_header, struct icmphdr **icmp_header)
{
    *ip_header = (struct iphdr *)buffer;   
    *icmp_header = (struct icmphdr *)(buffer + ((*ip_header)->ihl * 4));

    if (ntohs((*ip_header)->tot_len) < sizeof(struct iphdr))
        return FAILURE;
    return SUCCESS;
}