#include "socket.h"

int initSocketFd()
{
    struct protoent *proto = getprotobyname("icmp");
    if (proto == NULL)
    {
        perror("getprotobyname");
        exit(EXIT_FAILURE);
    }
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(IP_HDRINCL)");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int sendRequest(int sockfd, struct sockaddr_in *destAddress, t_packet *request)
{
    return sendto(sockfd, (void *)request->ip_hdr, ntohs(request->ip_hdr->tot_len), 0, (struct sockaddr *)destAddress, sizeof(struct sockaddr_in));
}

status socketIsReady(int sockfd, fd_set *readfds, struct timeval *timeout)
{
    FD_SET(sockfd, readfds);
    if (select(sockfd + 1, readfds, NULL, NULL, timeout) < 0)
        return FAILURE;
    if (timeout->tv_sec == 0 && timeout->tv_usec == 0)
        return FAILURE;
    FD_CLR(sockfd, readfds);
    return SUCCESS;
}