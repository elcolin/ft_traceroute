#include "ft_traceroute.h"

void triggerErrorNoFreeingIf(bool condition, char *msg, char *reason)
{
    if (condition)
    {
        fprintf(stderr, "%s: %s", msg, reason);
        exit(EXIT_FAILURE);
    }
}

int printPacketError(t_packet *reply, const int sequenceNumber)
{
    if (reply->ip_hdr == NULL || reply->icmp_hdr == NULL || reply->icmp_hdr->type == 0)
        return FALSE;
    uint32_t saddr = reply->ip_hdr->saddr; // in network byte order
    struct in_addr addr;
    addr.s_addr = saddr;

    char *ip_str = inet_ntoa(addr); // returns a pointer to static buffer
    switch (reply->icmp_hdr->type)
    {
        case ICMP_DEST_UNREACH:
        {
            char *msg_unreach[] = {"Destination net unreachable", "Destination host unreachable", "Destination protocol unreachable", "Destination port unreachable", "Fragmentation needed and DF set", "Source route failed"};
            printf("From %s icmp_seq=%d %s\n", ip_str, sequenceNumber, msg_unreach[reply->icmp_hdr->code]);
        }
        return TRUE;
        break;
        case ICMP_TIME_EXCEEDED:
        {
            char *msg_ttl[] = {"Time to live exceeded", "Fragment Reass time exceeded"};
            printf("From %s icmp_seq=%d %s\n", ip_str, sequenceNumber, msg_ttl[reply->icmp_hdr->code]);
        }
        return TRUE;
        break;
        default:
            return FALSE;
    }
    return FALSE;
}

void printPacket(t_packet *reply)
{
    if (reply == NULL)
    {
        printf("* ");
        return;
    }

    uint32_t saddr = reply->ip_hdr->saddr; // in network byte order
    struct in_addr addr;
    addr.s_addr = saddr;

    char *ip_str = inet_ntoa(addr);
    printf("%s\t", ip_str);
}

void printIPHeader(struct iphdr *reply)
{
    if (reply == NULL)
    {
        printf("* ");
        return;
    }

    uint32_t saddr = reply->saddr; // in network byte order
    struct in_addr addr;
    addr.s_addr = saddr;

    char *ip_str = inet_ntoa(addr);
    printf("%s  ", ip_str);
}

int main(int argc, char *argv[])
{
    int                 sockfd;
    fd_set              readfds;
    struct timeval      timeout;
    // long                rtt_microseconds = 0;

    t_packet    requestPacket = {};
    struct iphdr    replyPackets[MAX_HOPS * PACKET_NUMBER] = {};

    struct sockaddr_in  addrs[2] = {0};
    uint8_t             requestBuffer[1024] = {};
    uint8_t             replyBuffer[1024] = {};
    size_t              hops = 0;

    srand(time(NULL));
    if (argc < 2)
        return 1;//TO DO add more detailed error
    printf("%s %s\n", argv[0], argv[1]);
    setDestinationAddress(&addrs[DESTINATION], argv[1]);
    setSourceAddress(&addrs[SOURCE], &addrs[DESTINATION]);

    sockfd = initSocketFd();
    FD_ZERO(&readfds);
    while (hops < MAX_HOPS)
    {
        hops++;
        for (int i = 0; i < PACKET_NUMBER; i++)
        {
            initPacket((requestBuffer), &requestPacket);
            defineRequestIPHeader(requestPacket.ip_hdr,
                addrs[SOURCE].sin_addr.s_addr,
                addrs[DESTINATION].sin_addr.s_addr,
                hops,
                (getpid() + i) & 0xFFFF);
            defineRequestUDPHeader(requestPacket.udp_hdr);
            triggerErrorIf(sendRequest(sockfd, &addrs[DESTINATION], &requestPacket) < 0, "sendto failed", sockfd);
        }
        usleep(50);
    }
    hops = 0;
    t_packet replyPacket;
    (void) replyPackets;
    size_t pcknb = 0;
    while (hops < MAX_HOPS)
    {
        for (int i = 0; i < PACKET_NUMBER; i++)
        {
            timeout.tv_usec = 30000;
            timeout.tv_sec = 0;// TO DO change
            if (socketIsReady(sockfd, &readfds, &timeout) == FAILURE)
                continue;
            int size = receiveResponse((void *)replyBuffer, sockfd, sizeof(replyBuffer));
            triggerErrorIf(size < 0, "recvfrom failed", sockfd);
            if (parsePacket(replyBuffer, &replyPacket.ip_hdr, &replyPacket.icmp_hdr) == FAILURE)
                continue;
            struct udphdr      *errorPacketPtr = NULL;
            errorPacketPtr = (void *)IPHDR_SHIFT(ICMPHDR_SHIFT((replyPacket.icmp_hdr)));
            u_int16_t seq = ntohs(errorPacketPtr->uh_dport) - DEFAULT_DEST_PORT;
            if (seq == 0)
                continue;
            memcpy(&replyPackets[seq - 1], &(*replyPacket.ip_hdr), sizeof(struct iphdr));
            pcknb++;
        }
        hops ++;
    }
    hops = 0;
    struct iphdr tst = {};
    while (hops < MAX_HOPS && pcknb)
    {
        printf("%ld ", hops + 1);
        for (int i = 0; i < PACKET_NUMBER; i++)
        {
            if (memcmp(&replyPackets[hops * PACKET_NUMBER + i], &tst, sizeof(struct iphdr)))
            {
                pcknb--;
                printIPHeader(&replyPackets[hops * PACKET_NUMBER + i]);
            }
            else
                printf("* ");
        }
        printf("\n");
        hops ++;
    }
}