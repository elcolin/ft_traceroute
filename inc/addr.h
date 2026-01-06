#ifndef DOMAIN_H
#define DOMAIN_H

#include "utils.h"
#include <sys/socket.h>
// EXIT_FAILURE
#include <stdlib.h>
// getaddrinfo
#include <netdb.h>
// stderr
#include <stdio.h>
// inet_pton
#include <arpa/inet.h>
//memset
#include <string.h>

//ifaddrs
#include <sys/types.h>
#include <ifaddrs.h>

// IFF
#include <net/if.h>
#include <sys/ioctl.h>


status resolveFQDN(char *fqdn, struct sockaddr_in *addr);
status setDestinationAddress(struct sockaddr_in *destAddress, char *ip_address);
void setSourceAddress(struct sockaddr_in *srcAddress, u_int16_t familyType);

#endif