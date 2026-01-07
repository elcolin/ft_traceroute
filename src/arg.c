#include "arg.h"

void findHelpArgument(char **argv, int argc)
{
    for (int i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "--help"))
            printHelpAndExit();
    }
}

void printHelpAndExit()
{
    printf("Usage: \ntraceroute host [ packetlen ]\n");
    exit(EXIT_SUCCESS);
}


uint16_t setPacketLen(char *arg)
{
    int packetLen = getPacketLenNumberFromStr(arg);
    switch (packetLen) {
        case TOOBIG:
            fprintf(stderr, "too big packetlen %s specified\n", arg);
            exit(EXIT_FAILURE);
        break;
        case WRONGARG:
            errorArgumentHandler(2, arg);
        break;
    }
    return (uint16_t) packetLen;
}

void errorArgumentHandler(int pos, char *arg)
{
    static const char *errorStr[] = {"host", "packetlen"};
    fprintf(stderr, "Cannot handle \"%s\" cmdline arg `%s` on position %d (argc %d)\n", errorStr[pos - 1], arg, pos, pos);
    exit(EXIT_FAILURE);
}

int getPacketLenNumberFromStr(char *arg)
{
    int res = 0;
    for (size_t i = 0; i < strlen(arg); i++)
    {
        if(!isdigit(arg[i]))
            return WRONGARG;
    }
    res = atoi(arg);
    if (res >= BUFFER_SIZE)
        return TOOBIG;
    return res;
}