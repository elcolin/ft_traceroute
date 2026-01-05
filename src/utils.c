#include "utils.h"

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