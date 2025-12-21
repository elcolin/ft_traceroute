#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <sys/select.h>
// getprotobyname
#include <netdb.h>
// EXIT_FAILURE
#include <stdlib.h>
// perror
#include <stdio.h>
// close
#include <unistd.h>
#include "utils.h"
#include "packet.h"

int initSocketFd();
int sendRequest(int sockfd, struct sockaddr_in *destAddress, t_packet *request);
static inline int receiveResponse(void *buffer, int sockfd, u_int16_t buffer_size)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    return recvfrom(sockfd, buffer, buffer_size, 0, (struct sockaddr *)&addr, &addr_len);
};
status socketIsReadyToRead(int sockfd, fd_set *readfds, struct timeval *timeout);
status socketIsReadyToWrite(int sockfd, fd_set *writefds, struct timeval *timeout);
#endif
