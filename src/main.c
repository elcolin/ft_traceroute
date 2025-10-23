#include "ft_traceroute.h"

void triggerErrorNoFreeingIf(bool condition, char *msg, char *reason)
{
    if (condition)
    {
        fprintf(stderr, "%s: %s", msg, reason);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    int                 sockfd;
    fd_set              readfds;
    u_int16_t           sequenceNumber = 0;
    struct timeval      timeout;
    // long                rtt_microseconds = 0;
    // t_packet            probe = {};
    t_packet            requestPackets[PACKET_NUMBER] = {0};
    // t_packet            replyPaquets[3] = {0};
    struct sockaddr_in  addrs[2] = {0};
    // uint8_t             buffer[1024];

    if (argc < 2)
        return 1;//TO DO add more detailed error
    printf("%s %s\n", argv[0], argv[1]);
    setDestinationAddress(&addrs[DESTINATION], argv[1]);
    setSourceAddress(&addrs[SOURCE], &addrs[DESTINATION]);

    sockfd = initSocketFd();
    FD_ZERO(&readfds);
    while (sequenceNumber <= MAX_HOPS)
    {
        memset(&timeout, 0, sizeof(timeout));
        for (int i = 0; i < PACKET_NUMBER; i++)
        {
            initPacket(&requestPackets[i]);
            defineRequestPacket(&requestPackets[i], addrs[SOURCE].sin_addr.s_addr, addrs[DESTINATION].sin_addr.s_addr, ++sequenceNumber);
            triggerErrorIf(sendRequest(sockfd, &addrs[DESTINATION], &requestPackets[i]) < 0, "sendto failed", sockfd);
        }
        // int retrievedPackets = 0;
        // timeout.tv_sec = 1;// TO DO change
        // while (retrievedPackets < PACKET_NUMBER)
        // {
        //     if (socketIsReady(sockfd, &readfds, &timeout) == FAILURE)
        //         break;
        //     triggerErrorIf(receiveResponse((void *)buffer, sockfd, sizeof(buffer)) < 0, "recvfrom failed", sockfd);
        //     for (int i = 0; i < PACKET_NUMBER; i++)
        //     {
        //         if (getValidPacket(&packets[REPLY], &requestPackets[i]) == FAILURE)
        //         {
        //             if (printPacketError(&packets[REPLY], sequenceNumber) == TRUE)
        //                 break;
        //             continue;
        //         }
        //     }
    }
}