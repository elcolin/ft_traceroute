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

void triggerErrorNoFreeingIf(bool condition, char *msg, char *reason);
long get_elapsed_microseconds(struct timeval start, struct timeval end);
bool ipsAreEqual(u_int32_t *ip1, u_int32_t *ip2);



#endif