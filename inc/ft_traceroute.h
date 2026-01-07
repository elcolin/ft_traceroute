#ifndef FT_TRACEROUTE_H
#define FT_TRACEROUTE_H

#include "socket.h"
#include "addr.h"
#include "packet.h"
#include "utils.h"
// time
#include <time.h>
#include <sys/time.h>

#define NUMBER_OF_PROBES 3
#define MAX_HOPS 30
#define JITTER 30000
#define PACKET_NUMBER(x) ((x * NUMBER_OF_PROBES))

enum ADDR_TYPE{
    SOURCE,
    DESTINATION
};

static inline void triggerErrorIf(int condition, char *msg, int sockfd)
{
    if (condition)
    {
        perror(msg);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
};

void sendProbesToDestination(int sockfd, 
                            struct sockaddr_in addrs[2], 
                            struct timeval requestTimestamp[MAX_HOPS * NUMBER_OF_PROBES], 
                            const u_int16_t packetLen);
size_t receiveProbesFeedback(int sockfd, 
                            struct iphdr replyPackets[MAX_HOPS * NUMBER_OF_PROBES], 
                            struct timeval replyTimestamp[MAX_HOPS * NUMBER_OF_PROBES]);
void printResponses(const struct iphdr replyPackets[MAX_HOPS * NUMBER_OF_PROBES], 
                    const struct timeval requestTimestamp[MAX_HOPS * NUMBER_OF_PROBES], 
                    const struct timeval replyTimestamp[MAX_HOPS * NUMBER_OF_PROBES], 
                    const struct sockaddr_in addrs[2],
                    const size_t latestPacket);

#endif