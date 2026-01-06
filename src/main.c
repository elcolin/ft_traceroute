#include "ft_traceroute.h"

void argumentHandler(int pos, char *arg)
{
    static const char *errorStr[] = {"host", "packetlen"};
    fprintf(stderr, "Cannot handle \"%s\" cmdline arg `%s` on position %d (argc %d)\n", errorStr[pos - 1], arg, pos, pos);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int                 sockfd;
    struct iphdr        replyPackets[MAX_HOPS * NUMBER_OF_PROBES] = {};
    struct sockaddr_in  addrs[2] = {0};
    struct timeval      requestTimestamp[MAX_HOPS * NUMBER_OF_PROBES] = {};
    struct timeval      replyTimestamp[MAX_HOPS * NUMBER_OF_PROBES] = {};
    
    srand(time(NULL));
    if (argc < 2 || argc > 2)
        return 1;//TO DO add more detailed error
    if(setDestinationAddress(&addrs[DESTINATION], argv[1]) == FAILURE)
        argumentHandler(1, argv[1]);
    setSourceAddress(&addrs[SOURCE], addrs[DESTINATION].sin_family);
    printf("traceroute to %s (), %d hops max, byte packets\n", argv[1], MAX_HOPS);

    sockfd = initSocketFd();
    sendProbesToDestination(sockfd, addrs, requestTimestamp);
    receiveProbesFeedback(sockfd, replyPackets, replyTimestamp);
    close(sockfd);
    printResponses(replyPackets, requestTimestamp, replyTimestamp, addrs);
}