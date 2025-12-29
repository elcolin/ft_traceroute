#include "ft_traceroute.h"

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