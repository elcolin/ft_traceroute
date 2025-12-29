#include "ft_traceroute.h"

inline void printIPHeader(struct iphdr *reply)
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

void sendProbesToDestination(int sockfd, struct sockaddr_in addrs[2], struct timeval requestTimestamp[MAX_HOPS * PACKET_NUMBER])
{
    uint8_t             requestBuffer[BUFFER_SIZE] = {};
    t_udp_packet        requestPacket = {};
    struct timeval      timeout = {};
    fd_set              writefds;
    size_t              hops = 0;
    int                 bytesSent = 0;


    FD_ZERO(&writefds);
    while (hops < MAX_HOPS)
    {
        for (int i = 0; i < PACKET_NUMBER; i++)
        {
            timeout.tv_usec = JITTER;
            timeout.tv_sec = 0;
            memset(&requestBuffer, 0, bytesSent);
            initPacket((requestBuffer), &requestPacket);
            defineRequestIPHeader(requestPacket.ip_hdr,
                addrs[SOURCE].sin_addr.s_addr,
                addrs[DESTINATION].sin_addr.s_addr,
                hops + 1,
                (getpid() + (hops * PACKET_NUMBER) + i) & 0xFFFF);
            defineRequestUDPHeader(requestPacket.ip_hdr, requestPacket.udp_hdr);
            while (socketIsReadyToWrite(sockfd, &writefds, &timeout))
                continue;
            gettimeofday(&requestTimestamp[hops * PACKET_NUMBER + i], NULL);
            bytesSent = sendRequest(sockfd, &addrs[DESTINATION], &requestPacket);
            triggerErrorIf(bytesSent < 0, "sendto failed", sockfd);
        }
        hops++;
    }
}

void receiveProbesFeedback(int sockfd, struct iphdr replyPackets[MAX_HOPS * PACKET_NUMBER], struct timeval replyTimestamp[MAX_HOPS * PACKET_NUMBER])
{
    size_t              hops = 0;
    struct timeval      timeout = {};
    u_int8_t            replyBuffer[BUFFER_SIZE] = {};
    int                 bytesReceived = 0;
    fd_set              readfds;
    t_icmp_packet       replyPacket;

    FD_ZERO(&readfds);
    while (hops < MAX_HOPS)
    {
        for (int i = 0; i < PACKET_NUMBER; i++)
        {
            timeout.tv_usec = JITTER;
            timeout.tv_sec = 0;// TO DO change
            memset(replyBuffer, 0, bytesReceived);
            if (socketIsReadyToRead(sockfd, &readfds, &timeout) == FAILURE) // If Timeout
                continue;
            bytesReceived = receiveResponse((void *)replyBuffer, sockfd, sizeof(replyBuffer));
            triggerErrorIf(bytesReceived < 0, "recvfrom failed", sockfd);
            if (parsePacket(replyBuffer, &replyPacket.ip_hdr, &replyPacket.icmp_hdr) == FAILURE) // If no valid packet
                continue;
            struct udphdr      *errorPacketPtr = NULL;
            errorPacketPtr = (void *)IPHDR_SHIFT(ICMPHDR_SHIFT((replyPacket.icmp_hdr)));
            u_int16_t seq = ntohs(errorPacketPtr->uh_dport) - DEFAULT_DEST_PORT + 1;
            if (seq <= 0)
                continue;
            gettimeofday(&replyTimestamp[seq - 1], NULL);
            memcpy(&replyPackets[seq - 1], &(*replyPacket.ip_hdr), sizeof(struct iphdr));
        }
        hops ++;
    }
}

void printResponses(struct iphdr replyPackets[MAX_HOPS * PACKET_NUMBER], struct timeval requestTimestamp[MAX_HOPS * PACKET_NUMBER], struct timeval replyTimestamp[MAX_HOPS * PACKET_NUMBER])
{
    size_t              hops = 0;
    struct iphdr tst = {};
    long  rtt_microseconds = 0;

    while (hops < MAX_HOPS)
    {
        printf("%ld ", hops + 1);
        for (int i = 0; i < PACKET_NUMBER; i++)
        {
            if (memcmp(&replyPackets[hops * PACKET_NUMBER + i], &tst, sizeof(struct iphdr)))
                printIPHeader(&replyPackets[hops * PACKET_NUMBER + i]);
            else
                printf("* ");
        }
        for (int i = 0; i < PACKET_NUMBER; i++)
        {
            if (memcmp(&replyPackets[hops * PACKET_NUMBER + i], &tst, sizeof(struct iphdr)))
            {
                rtt_microseconds = get_elapsed_microseconds(requestTimestamp[hops * PACKET_NUMBER + i], replyTimestamp[hops * PACKET_NUMBER + i]);
                printf("%.3f ms  ", rtt_microseconds / 1000.0);
            }
        }
        printf("\n");
        hops ++;
    }
}

int main(int argc, char *argv[])
{
    int                 sockfd;
    struct iphdr        replyPackets[MAX_HOPS * PACKET_NUMBER] = {};
    struct sockaddr_in  addrs[2] = {0};
    struct timeval      requestTimestamp[MAX_HOPS * PACKET_NUMBER] = {};
    struct timeval      replyTimestamp[MAX_HOPS * PACKET_NUMBER] = {};

    srand(time(NULL));
    if (argc < 2)
        return 1;//TO DO add more detailed error
    printf("%s %s\n", argv[0], argv[1]);
    setDestinationAddress(&addrs[DESTINATION], argv[1]);
    setSourceAddress(&addrs[SOURCE], addrs[DESTINATION].sin_family);

    sockfd = initSocketFd();
    sendProbesToDestination(sockfd, addrs, requestTimestamp);
    receiveProbesFeedback(sockfd, replyPackets, replyTimestamp);
    close(sockfd);
    printResponses(replyPackets, requestTimestamp, replyTimestamp);
}