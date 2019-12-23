#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cobj.h"

struct APIGetMobileSecretKeyMsg {
	CSTR esn;
    CSTR timestamp;
	CSTR password;
    CSTR jsonStr;
};
DEFINE_METAINFO(APIGetMobileSecretKeyMsg);

struct APICeCmdCallbackMsg {
    CSTR esn;
    CSTR timestamp;
    CSTR jsonStr;
    CSTR signature;
};
DEFINE_METAINFO(APICeCmdCallbackMsg);

struct APICeEchoMsg {
    CSTR esn;
    CSTR timestamp;
    CSTR jsonStr;
    CSTR signature;
};
DEFINE_METAINFO(APICeEchoMsg);

struct APICeStartupMsg {
    CSTR esn;
    CSTR timestamp;
    CSTR jsonStr;
    CSTR signature;
};
DEFINE_METAINFO(APICeStartupMsg);

struct APIConnectVpeMsg {
    CSTR esn;
    CSTR timestamp;
    CSTR jsonStr;
    //CSTR data;
    int timeout;
    //CSTR osVersion;
    CSTR signature;
};
DEFINE_METAINFO(APIConnectVpeMsg);

char * assemble_msg(char *msg_name, char *api_msg);
