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

// void defineRequestPacket(t_packet *request,
//                         uint32_t src_ip,
//                         uint32_t dst_ip,
//                         uint8_t sequenceNumber,
//                         uint8_t ttl)
// {
//     //defineRequestICMPHeader(request->icmp_hdr, sequenceNumber);
//     //defineRequestIPHeader(request->ip_hdr, src_ip, dst_ip, sequenceNumber, ttl);    
// }

// status comparePackets(struct icmphdr *icmp_reply, struct icmphdr *icmp_request)
// {
//     if (icmp_reply == NULL || icmp_request == NULL)
//         return FAILURE;
//     if (ntohs(icmp_reply->un.echo.id) != ntohs(icmp_request->un.echo.id))
//     // Not the same process
//         return FAILURE;
//     if (ntohs(icmp_reply->un.echo.sequence) != ntohs(icmp_request->un.echo.sequence))
//     // Not the same sequence id as request
//         return FAILURE;
//     return SUCCESS;
// }

int parsePacket(void *buffer, struct iphdr **ip_header, struct icmphdr **icmp_header)
{
    *ip_header = (struct iphdr *)buffer;   
    *icmp_header = (struct icmphdr *)(buffer + ((*ip_header)->ihl * 4));
    if (ntohs((*ip_header)->tot_len) < sizeof(struct iphdr))
        return -1;
    return ntohs((*ip_header)->tot_len);
}

status getValidPacket(u_int8_t *buffer, t_packet *reply, int size)
{
    int pkg_idx = 0;
    while (pkg_idx < (int) size && pkg_idx > -1)
    {
        // printf("size %d\t", size);
        // Loop until we find a valid packet
        pkg_idx = parsePacket((buffer + pkg_idx), &reply->ip_hdr, &reply->icmp_hdr);
        if (reply->icmp_hdr && reply->icmp_hdr->type)
        {
            //struct icmphdr *errorPacket = (void *)IPHDR_SHIFT(ICMPHDR_SHIFT(reply->icmp_hdr));
            return SUCCESS;
        }
    }
    return FAILURE;
}