#include "ft_traceroute.h"

#include <ctype.h>

void errorArgumentHandler(int pos, char *arg)
{
    static const char *errorStr[] = {"host", "packetlen"};
    fprintf(stderr, "Cannot handle \"%s\" cmdline arg `%s` on position %d (argc %d)\n", errorStr[pos - 1], arg, pos, pos);
    exit(EXIT_FAILURE);
}
typedef enum {
    TOOBIG = -2,
    WRONGARG = -3
} packetLenError;

int getPacketLenNumberFromStr(char *arg)
{
    int res = 0;
    for (size_t i = 0; i < strlen(arg); i++)
    {
        if(!isdigit(arg[i]))
            return WRONGARG;
    }
    res = atoi(arg);
    if (res >= BUFFER_SIZE)
        return TOOBIG;
    return res;
}

uint16_t setPacketLen(char *arg)
{
    int packetLen = getPacketLenNumberFromStr(arg);
    switch (packetLen) {
        case TOOBIG:
            fprintf(stderr, "too big packetlen %s specified\n", arg);
            exit(EXIT_FAILURE);
        break;
        case WRONGARG:
            errorArgumentHandler(2, arg);
        break;
    }
    return (uint16_t) packetLen;
}

void printHelpAndExit()
{
    printf("Usage: \ntraceroute host [ packetlen ]\n");
    exit(EXIT_SUCCESS);
}

void findHelpArgument(char **argv, int argc)
{
    for (int i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "--help"))
            printHelpAndExit();
    }
}


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