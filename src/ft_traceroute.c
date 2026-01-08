#include "ft_traceroute.h"

void sendProbesToDestination(int sockfd, struct sockaddr_in addrs[2], struct timeval requestTimestamp[MAX_HOPS * NUMBER_OF_PROBES], const u_int16_t packetLen)
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
                getpid() & 0xFFFF,
                packetLen);
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

inline size_t receiveProbesFeedback(int sockfd, 
                        struct iphdr replyPackets[MAX_HOPS * NUMBER_OF_PROBES],
                        struct timeval replyTimestamp[MAX_HOPS * NUMBER_OF_PROBES])
{
    static size_t              hops = 0;
    static u_int8_t            replyBuffer[BUFFER_SIZE] = {};
    int                 bytesReceived = 0;
    static fd_set              readfds;
    t_icmp_packet       replyPacket;
    static struct timeval      timeout = {};
    static struct timeval      startTime = {};
    static struct timeval      currentTime = {};
    static struct udphdr       *feedbackUdpPtr = NULL;

    FD_ZERO(&readfds);
    gettimeofday(&startTime, NULL);
    while (hops < MAX_HOPS)
    {
        for (int i = 0; i < NUMBER_OF_PROBES; i++)
        {
            timeout.tv_usec = 330000;
            timeout.tv_sec = 0;
            if (socketIsReadyToRead(sockfd, &readfds, &timeout) == FAILURE) // If Timeout
            continue;
            bytesReceived = receiveResponse((void *)replyBuffer, sockfd, sizeof(replyBuffer));
            triggerErrorIf(bytesReceived < 0, "recvfrom failed", sockfd);
            size_t packetIdx = 0;
            while (findValidPacket(&replyBuffer[packetIdx], &replyPacket.ip_hdr, &replyPacket.icmp_hdr, bytesReceived) == SUCCESS)
            {
                feedbackUdpPtr = (void *)IPHDR_SHIFT(ICMPHDR_SHIFT((replyPacket.icmp_hdr)));
                u_int16_t seq = ntohs(feedbackUdpPtr->uh_dport) - DEFAULT_DEST_PORT;
                gettimeofday(&replyTimestamp[seq], NULL);
                memcpy(&replyPackets[seq], &(*replyPacket.ip_hdr), sizeof(struct iphdr));
                memset(replyBuffer, 0, bytesReceived);
                packetIdx += replyPacket.ip_hdr->tot_len;
            }
        }
        gettimeofday(&currentTime, NULL);
        hops ++;
        if (get_elapsed_microseconds(startTime, currentTime) > 5000000)
        break;
    }
    return PACKET_NUMBER(hops);
}

inline void printResponses(const struct iphdr replyPackets[MAX_HOPS * NUMBER_OF_PROBES], 
                    const struct timeval requestTimestamp[MAX_HOPS * NUMBER_OF_PROBES], 
                    const struct timeval replyTimestamp[MAX_HOPS * NUMBER_OF_PROBES], 
                    const struct sockaddr_in addrs[2],
                    const size_t latestPacket)
{
    static size_t  hops = 0;
    size_t  isDestination = 0;
    const   struct  iphdr tst = {};
    long    rtt_microseconds = 0;
    bool    hopHasBeenPrinted[NUMBER_OF_PROBES] = {0};

    while (PACKET_NUMBER(hops) < latestPacket)
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
            for (int j = i; j < NUMBER_OF_PROBES; j++) // Print all the probes tied to the IP address of the packet
            {
                if(i != j && ipsAreEqual(&replyPackets[PACKET_NUMBER(hops) + i].saddr, &replyPackets[PACKET_NUMBER(hops) + j].saddr) == FALSE)
                    continue;
                if (ipsAreEqual((uint32_t *) &addrs[DESTINATION].sin_addr, &replyPackets[PACKET_NUMBER(hops) + j].saddr) == TRUE)
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