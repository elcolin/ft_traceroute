#ifndef FT_TRACEROUTE_H
#define FT_TRACEROUTE_H

#include "socket.h"
#include "addr.h"
#include "packet.h"

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
}

#endif