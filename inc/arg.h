#ifndef ARG_H
#define ARG_H
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "packet.h"

typedef enum {
    TOOBIG = -2,
    WRONGARG = -3
} packetLenError;

void findHelpArgument(char **argv, int argc);
void printHelpAndExit();
uint16_t setPacketLen(char *arg);
void errorArgumentHandler(int pos, char *arg);
int getPacketLenNumberFromStr(char *arg);

#endif