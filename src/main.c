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


int main(int argc, char *argv[])
{
    int                 sockfd;
    fd_set              readfds;
    u_int16_t           sequenceNumber = 1;
    struct timeval      timeout;
    // long                rtt_microseconds = 0;
    // t_packet            probe = {};
    t_packet            requestPackets[PACKET_NUMBER] = {0};
    t_packet            *replyPackets[PACKET_NUMBER] = {0};
    t_packet            errorPacket;
    struct icmphdr       *errorPacketPtr = NULL;
    struct sockaddr_in  addrs[2] = {0};
    uint8_t             requestBuffer[1024] = {};
    uint8_t             replyBuffer[1024] = {};
    size_t              hops = 0;

    if (argc < 2)
        return 1;//TO DO add more detailed error
    printf("%s %s\n", argv[0], argv[1]);
    setDestinationAddress(&addrs[DESTINATION], argv[1]);
    setSourceAddress(&addrs[SOURCE], &addrs[DESTINATION]);

    sockfd = initSocketFd();
    FD_ZERO(&readfds);
    while (hops <= MAX_HOPS)
    {
        memset(&timeout, 0, sizeof(timeout));
        memset(&replyBuffer, 0, sizeof(replyBuffer));
        memset(&requestBuffer, 0, sizeof(requestBuffer));
        uint16_t shift = 0;
        hops++;
        for (int i = 0; i < PACKET_NUMBER; i++)
        {
            initPacket((requestBuffer + shift), &requestPackets[i]);
            //defineRequestPacket(&requestPackets[i], addrs[SOURCE].sin_addr.s_addr, addrs[DESTINATION].sin_addr.s_addr, sequenceNumber, hops);
            defineRequestIPHeader(requestPackets[i].ip_hdr,
                           addrs[SOURCE].sin_addr.s_addr,
                           addrs[DESTINATION].sin_addr.s_addr,
                           hops,
                           (getpid() + i) & 0xFFFF);
            defineRequestICMPHeader(requestPackets[i].icmp_hdr, getpid() & 0xFFFF, sequenceNumber++);
            // printf("mount2 %d \n", ntohs(requestPackets[i].icmp_hdr->un.echo.id));
            
            triggerErrorIf(sendRequest(sockfd, &addrs[DESTINATION], &requestPackets[i]) < 0, "sendto failed", sockfd);
            //usleep(50);

            shift += ntohs(requestPackets[i].ip_hdr->tot_len);

        }
        memset(replyPackets, 0, sizeof(replyPackets));

        while (1)
        {
            timeout.tv_usec = 50000 + sequenceNumber * 10000;
            timeout.tv_sec = 0;// TO DO change
            if (socketIsReady(sockfd, &readfds, &timeout) == FAILURE)
                break;
            int size = receiveResponse((void *)replyBuffer, sockfd, sizeof(replyBuffer));
            triggerErrorIf(size < 0, "recvfrom failed", sockfd);
            shift = 0;
            // printf("s %d\n", size);
            while (getValidPacket((replyBuffer + shift), &errorPacket, size - shift) == SUCCESS)
            {
                errorPacketPtr = (void *)IPHDR_SHIFT(ICMPHDR_SHIFT((errorPacket.icmp_hdr)));
                for (int i = 0; i < PACKET_NUMBER; i++)
                {
                    if (replyPackets[i])
                        continue;
                    // printf("%d rec %d : req %d\n", i, ntohs(errorPacketPtr->un.echo.id), ntohs(requestPackets[i].icmp_hdr->un.echo.id));
                    if (errorPacketPtr->un.echo.sequence == requestPackets[i].icmp_hdr->un.echo.sequence)
                    {
                        replyPackets[i] = &errorPacket;
                        break;
                    }
                }
                shift += errorPacket.ip_hdr->tot_len;
            }
        }
        printf("%ld ", hops);
        for (int i = 0; i < PACKET_NUMBER; i++)
        {
            printPacket(replyPackets[i]);
        }
        printf("\n");
        usleep(50);
    }
}