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

void sendProbesToDestination(int sockfd, struct sockaddr_in addrs[2], struct timeval requestTimestamp[MAX_HOPS * NUMBER_OF_PROBES])
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
        for (int i = 0; i < NUMBER_OF_PROBES; i++)
        {
            timeout.tv_usec = JITTER;
            timeout.tv_sec = 0;
            memset(&requestBuffer, 0, bytesSent);
            initPacket((requestBuffer), &requestPacket);
            defineRequestIPHeader(requestPacket.ip_hdr,
                addrs[SOURCE].sin_addr.s_addr,
                addrs[DESTINATION].sin_addr.s_addr,
                hops + 1,
                (getpid() + (PACKET_NUMBER(hops)) + i) & 0xFFFF);
            defineRequestUDPHeader(requestPacket.ip_hdr, requestPacket.udp_hdr);
            while (socketIsReadyToWrite(sockfd, &writefds, &timeout) == FAILURE)
                continue;
            gettimeofday(&requestTimestamp[PACKET_NUMBER(hops) + i], NULL);
            bytesSent = sendRequest(sockfd, &addrs[DESTINATION], &requestPacket);
            triggerErrorIf(bytesSent < 0, "sendto failed", sockfd);
        }
        hops++;
    }
}

void receiveProbesFeedback(int sockfd, struct iphdr replyPackets[MAX_HOPS * NUMBER_OF_PROBES], struct timeval replyTimestamp[MAX_HOPS * NUMBER_OF_PROBES])
{
    size_t              hops = 0;
    struct timeval      timeout = {};
    u_int8_t            replyBuffer[BUFFER_SIZE] = {};
    int                 bytesReceived = 0;
    fd_set              readfds;
    t_icmp_packet       replyPacket;
    struct udphdr      *errorPacketPtr = NULL;

    FD_ZERO(&readfds);
    while (hops < MAX_HOPS)
    {
        for (int i = 0; i < NUMBER_OF_PROBES; i++)
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
            errorPacketPtr = (void *)IPHDR_SHIFT(ICMPHDR_SHIFT((replyPacket.icmp_hdr)));
            u_int16_t seq = ntohs(errorPacketPtr->uh_dport) - DEFAULT_DEST_PORT; //
            gettimeofday(&replyTimestamp[seq], NULL);
            memcpy(&replyPackets[seq], &(*replyPacket.ip_hdr), sizeof(struct iphdr));
        }
        hops ++;
    }
}

void printResponses(struct iphdr replyPackets[MAX_HOPS * NUMBER_OF_PROBES], 
                    struct timeval requestTimestamp[MAX_HOPS * NUMBER_OF_PROBES], 
                    struct timeval replyTimestamp[MAX_HOPS * NUMBER_OF_PROBES], 
                    struct sockaddr_in addrs[2])
{
    size_t  hops = 0;
    struct  iphdr tst = {};
    long    rtt_microseconds = 0;
    bool    hopHasBeenPrinted[NUMBER_OF_PROBES] = {0};
    size_t  isDestination = 0;

    while (hops < MAX_HOPS)
    {
        isDestination = 0;
        printf("%ld ", hops + 1);
        for (int i = 0; i < NUMBER_OF_PROBES; i++)
        {
            if (!memcmp(&replyPackets[PACKET_NUMBER(hops) + i], &tst, sizeof(struct iphdr)))
            {
                printf("* ");
                continue;
            }
            if (hopHasBeenPrinted[i] == TRUE)
                continue;
            printIPHeader(&replyPackets[PACKET_NUMBER(hops) + i]);
            for (int j = i; j < NUMBER_OF_PROBES; j++)
            {
                if(ipsAreEqual(&replyPackets[PACKET_NUMBER(hops) + i].saddr, &replyPackets[PACKET_NUMBER(hops) + j].saddr) == FALSE)
                    continue;
                if (!memcmp(&addrs[DESTINATION].sin_addr, &replyPackets[PACKET_NUMBER(hops) + j].saddr, sizeof(u_int32_t)))
                    isDestination++;
                rtt_microseconds = get_elapsed_microseconds(requestTimestamp[PACKET_NUMBER(hops) + j], replyTimestamp[PACKET_NUMBER(hops) + j]);
                printf("%.3f ms  ", rtt_microseconds / 1000.0);
                hopHasBeenPrinted[j] = TRUE;
            }
        }
        memset(&hopHasBeenPrinted, FALSE, NUMBER_OF_PROBES * sizeof(bool));
        printf("\n");
        hops ++;
        if (isDestination == NUMBER_OF_PROBES)
            return;        
    }
}