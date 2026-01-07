#include "utils.h"

inline void printIPHeader(const struct iphdr *reply)
{
    if (reply == NULL)
        return;

    uint32_t saddr = reply->saddr; // in network byte order
    struct in_addr addr;
    addr.s_addr = saddr;

    char *ip_str = inet_ntoa(addr);
    printf("(%s)  ", ip_str);
}

inline long get_elapsed_microseconds(const struct timeval start, const struct timeval end)
{
    return (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
}

inline bool ipsAreEqual(const u_int32_t *ip1, const u_int32_t *ip2)
{
    if (memcmp(ip1, ip2, sizeof(uint32_t)))
        return FALSE;
    return TRUE;
}


void triggerErrorNoFreeingIf(const bool condition, const char *msg, const char *reason)
{
    if (condition)
    {
        fprintf(stderr, "%s: %s", msg, reason);
        exit(EXIT_FAILURE);
    }
}