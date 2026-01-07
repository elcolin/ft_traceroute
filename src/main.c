#include "ft_traceroute.h"

int main(int argc, char *argv[])
{
    int                 sockfd;
    struct iphdr        replyPackets[MAX_HOPS * NUMBER_OF_PROBES] = {};
    struct sockaddr_in  addrs[2] = {0};
    struct timeval      requestTimestamp[MAX_HOPS * NUMBER_OF_PROBES] = {};
    struct timeval      replyTimestamp[MAX_HOPS * NUMBER_OF_PROBES] = {};
    int                 packetLen = 0;
    const int           minPacketLen = (int) (sizeof(struct iphdr) + sizeof(struct udphdr));

    srand(time(NULL));
    if (argc < 2)
        printHelpAndExit();
    findHelpArgument(argv, argc);

    if (argc > 3)
        fprintf(stderr, "Extra arg `%s' (position 3, argc 3)", argv[3]);
    if(setDestinationAddress(&addrs[DESTINATION], argv[1]) == FAILURE)
        errorArgumentHandler(1, argv[1]);
    if (argc == 3)
        packetLen = setPacketLen(argv[2]);
    packetLen = packetLen > minPacketLen ? packetLen : minPacketLen;

    setSourceAddress(&addrs[SOURCE], addrs[DESTINATION].sin_family);
    printf("traceroute to %s (%s), %d hops max, %d byte packets\n", argv[1], inet_ntoa(addrs[DESTINATION].sin_addr), MAX_HOPS, packetLen);

    sockfd = initSocketFd();
    sendProbesToDestination(sockfd, addrs, requestTimestamp, (uint16_t) packetLen);
    receiveProbesFeedback(sockfd, replyPackets, replyTimestamp);
    close(sockfd);
    printResponses(replyPackets, requestTimestamp, replyTimestamp, addrs);
}