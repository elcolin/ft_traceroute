#include "utils.h"

inline void printIPHeader(struct iphdr *reply)
{
    if (reply == NULL)
    {
        printf("* ");
        return;
    }

    uint32_t saddr = reply->saddr; // in network byte order
    struct in_addr addr;
    addr.s_addr = saddr;

    char *ip_str = inet_ntoa(addr);
    printf("(%s)  ", ip_str);
}

inline long get_elapsed_microseconds(struct timeval start, struct timeval end)
{
    return (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
}

inline bool ipsAreEqual(u_int32_t *ip1, u_int32_t *ip2)
{
    if (memcmp(ip1, ip2, sizeof(uint32_t)))
        return FALSE;
    return TRUE;
}


void triggerErrorNoFreeingIf(bool condition, char *msg, char *reason)
{
    if (condition)
    {
        fprintf(stderr, "%s: %s", msg, reason);
        exit(EXIT_FAILURE);
    }
}