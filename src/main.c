#include "ft_traceroute.h"

void triggerErrorNoFreeingIf(bool condition, char *msg, char *reason)
{
    if (condition)
    {
        fprintf(stderr, "%s: %s", msg, reason);
        exit(EXIT_FAILURE);
    }
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
    fd_set              readfds, writefds;
    struct timeval      timeout;
    // long                rtt_microseconds = 0;

    t_udp_packet    requestPacket = {};
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
    FD_ZERO(&writefds);
    while (hops < MAX_HOPS)
    {
        for (int i = 0; i < PACKET_NUMBER; i++)
        {
            timeout.tv_usec = 30000;
            timeout.tv_sec = 0;
            memset(&requestBuffer, 0, sizeof(requestBuffer));
            initPacket((requestBuffer), &requestPacket);
            defineRequestIPHeader(requestPacket.ip_hdr,
                addrs[SOURCE].sin_addr.s_addr,
                addrs[DESTINATION].sin_addr.s_addr,
                hops + 1,
                (getpid() + (hops * PACKET_NUMBER) + i) & 0xFFFF);
            defineRequestUDPHeader(requestPacket.ip_hdr, requestPacket.udp_hdr);
            while (socketIsReadyToWrite(sockfd, &writefds, &timeout))
                continue;
            triggerErrorIf(sendRequest(sockfd, &addrs[DESTINATION], &requestPacket) < 0, "sendto failed", sockfd);
        }
        hops++;
    }
    hops = 0;
    t_icmp_packet replyPacket;
    size_t pcknb = 0;
    while (hops < MAX_HOPS)
    {
        for (int i = 0; i < PACKET_NUMBER; i++)
        {
            timeout.tv_usec = 30000;
            timeout.tv_sec = 0;// TO DO change
            if (socketIsReadyToRead(sockfd, &readfds, &timeout) == FAILURE)
                continue;
            int size = receiveResponse((void *)replyBuffer, sockfd, sizeof(replyBuffer));
            triggerErrorIf(size < 0, "recvfrom failed", sockfd);
            if (parsePacket(replyBuffer, &replyPacket.ip_hdr, &replyPacket.icmp_hdr) == FAILURE)
                continue;
            struct udphdr      *errorPacketPtr = NULL;
            errorPacketPtr = (void *)IPHDR_SHIFT(ICMPHDR_SHIFT((replyPacket.icmp_hdr)));
            u_int16_t seq = ntohs(errorPacketPtr->uh_dport) - DEFAULT_DEST_PORT + 1;
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