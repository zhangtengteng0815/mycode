#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <windows.h>
#include <process.h>
#include "cJSON/cJSON.h"
#include "cobj.h"
#include "client.h"

#pragma comment(lib, "ws2_32.lib")

#define ICMP_ECHOREPLY 0
#define ICMP_ECHOREQ 8
#define REQ_DATASIZE 32

typedef struct PingIpsResults
{
    char *target;
    u_int transmitted;
    u_int received;
    u_int time;
    u_int loss;
    double min;
    double avg;
    double max;
    double deviation;
};


typedef struct IPHeader {
    u_char VIHL;
    u_char ToS;
    u_short TotalLen;
    u_short ID;
    u_short Frag_Flags;
    u_char TTL;
    u_char Protocol;
    u_short Checksum;
    struct in_addr SrcIP;
    struct in_addr DestIP;
} IPHDR, *PIPHDR;

typedef struct ICMPHeader {
    u_char Type;
    u_char Code;
    u_short Checksum;
    u_short ID;
    u_short Seq;
    char Data;
} ICMPHDR, *PICMPHDR;

typedef struct ECHOREQUEST {
    ICMPHDR icmpHdr;
    DWORD dwTime;
    char cData[REQ_DATASIZE];
} ECHOREQUEST, *PECHOREQUEST;

typedef struct ECHOREPLY {
    IPHDR ipHdr;
    ECHOREQUEST echoRequest;
    char cFiller[256];
} ECHOREPLY, *PECHOREPLY;


DWORD WINAPI Ping(LPVOID lpParam);
char * do_ping();
