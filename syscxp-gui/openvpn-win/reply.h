#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "cobj.h"
#include "cJSON/cJSON.h"

struct APIGetMobileSecretKeyReply {
    CSTR esn;
    CSTR secretKey;
    CSTR success;
};

struct APICeCmdCallbackReply {
    CSTR error;
    CSTR success;
};

struct APICeEchoReply {
    CSTR success;
    CSTR esn;
    CSTR deleted;
    CSTR finalized;
    CSTR cmd;
};

struct APICeStartupReply {
    CSTR success;
    CSTR esn;
    CSTR state;
    CSTR configStatus;
    CSTR vpeIps;
    CSTR masterLocalIp;
    CSTR slaveLocalIp;
    CSTR masterGateway;
    CSTR slaveGateway;
    CSTR tap;
    CSTR serviceCidrs;
    CSTR secretKey;
    int bandwidth;
};

struct APIConnectVpeEvent {
    CSTR success;
    CSTR esn;
    CSTR error;
};

struct Cmd {
    CSTR uuid;
    CSTR cmd;
    CSTR cmdData;
};

struct CmdJson {
    CSTR cmdUuid;
    CSTR jsonData;
    BOOL success;
    CSTR errMsg;
};

DEFINE_METAINFO(APICeCmdCallbackReply);
DEFINE_METAINFO(APICeEchoReply);
DEFINE_METAINFO(APIGetMobileSecretKeyReply);
DEFINE_METAINFO(APICeStartupReply);
DEFINE_METAINFO(APIConnectVpeEvent);
DEFINE_METAINFO(Cmd);
DEFINE_METAINFO(CmdJson);

char * unassemble_reply(char *reply_name, char *api_reply_body);
int is_error_reply(char *reply);
