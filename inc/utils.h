#ifndef UTILS_H
#define UTILS_H

//abs
#include <stdlib.h>
#include <stdio.h>

//time.h
#include <sys/time.h>
//uint64_t
#include <stdint.h>
//uint32_t
#include <string.h>
//inet_ntoa
#include <arpa/inet.h>
// ip header
#include <netinet/ip.h>


typedef enum
{
   FALSE,
   TRUE
}  bool;

typedef enum
{
   SUCCESS,
   FAILURE
}  status;

void printIPHeader(const struct iphdr *reply);
void triggerErrorNoFreeingIf(const bool condition, const char *msg, const char *reason);
long get_elapsed_microseconds(const struct timeval start, const struct timeval end);
bool ipsAreEqual(const u_int32_t *ip1, const u_int32_t *ip2);



#endif