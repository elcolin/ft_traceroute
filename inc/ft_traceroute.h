#ifndef FT_TRACEROUTE_H
#define FT_TRACEROUTE_H

#include "socket.h"
#include "addr.h"
#include "packet.h"
#include "utils.h"
// time
#include <time.h>
#include <sys/time.h>

#define PACKET_NUMBER 3
#define MAX_HOPS 30
#define JITTER 30000

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

void printIPHeader(struct iphdr *reply);
void sendProbesToDestination(int sockfd, struct sockaddr_in addrs[2], struct timeval requestTimestamp[MAX_HOPS * PACKET_NUMBER]);
void receiveProbesFeedback(int sockfd, struct iphdr replyPackets[MAX_HOPS * PACKET_NUMBER], struct timeval replyTimestamp[MAX_HOPS * PACKET_NUMBER]);
void printResponses(struct iphdr replyPackets[MAX_HOPS * PACKET_NUMBER], struct timeval requestTimestamp[MAX_HOPS * PACKET_NUMBER], struct timeval replyTimestamp[MAX_HOPS * PACKET_NUMBER]);

#endif