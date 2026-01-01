#include "addr.h"

status resolveFQDN(char *fqdn, struct sockaddr_in *addr)
{
    struct addrinfo hints;
    struct addrinfo *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_RAW;
    int error = getaddrinfo(fqdn, NULL, &hints, &result);
    if (error != 0)
    {
        fprintf(stderr, "getaddrinfo failed: %s", gai_strerror(error));
        exit(EXIT_FAILURE);
    }
    memcpy(addr, result->ai_addr, sizeof(struct sockaddr_in));
    freeaddrinfo(result);
    return SUCCESS;
}

void setDestinationAddress(struct sockaddr_in *destAddress, char *ip_address)
{
    memset(destAddress, 0, sizeof(*destAddress));
    destAddress->sin_family = AF_INET;
    if(inet_pton(PF_INET, ip_address, &destAddress->sin_addr) != 1 && resolveFQDN(ip_address, destAddress) == FAILURE)
    {
        fprintf(stderr, "Invalid address: %s\n", ip_address);
        exit(EXIT_FAILURE);
    }
}

void setSourceAddress(struct sockaddr_in *srcAddress, u_int16_t familyType)
{
    memset(srcAddress, 0, sizeof(*srcAddress));
    struct ifaddrs *networkInterfaces = NULL;
    
    if (getifaddrs(&networkInterfaces) != 0)
    {
        perror("getifaddrs failed");
        exit(EXIT_FAILURE);
    };
    struct ifaddrs *idx = networkInterfaces;
    while (idx)
    {
        if ((idx->ifa_addr->sa_family == familyType) 
            && !(idx->ifa_flags & IFF_LOOPBACK) 
            && (idx->ifa_flags & IFF_BROADCAST) 
            && (idx->ifa_flags & IFF_UP))
        {
            memcpy(srcAddress, idx->ifa_addr, sizeof(struct sockaddr_in));
            freeifaddrs(networkInterfaces);
            return;
        }
        idx = idx->ifa_next;
    }
    freeifaddrs(networkInterfaces);
    fprintf(stderr, "Coulnd't get host address");
    exit(EXIT_FAILURE);
}