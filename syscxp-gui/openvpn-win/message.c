#include "message.h"
char msgheader[] = "{\"com.syscxp.header.sdwan.ce.daemon.mobile.";

DEFINE_METAINFO(APIGetMobileSecretKeyMsg)
{
    METAINFO_CREATE(APIGetMobileSecretKeyMsg);

    METAINFO_ADD_MEMBER(APIGetMobileSecretKeyMsg, FIELD_TYPE_CSTR, esn);
    METAINFO_ADD_MEMBER(APIGetMobileSecretKeyMsg, FIELD_TYPE_CSTR, timestamp);
    METAINFO_ADD_MEMBER(APIGetMobileSecretKeyMsg, FIELD_TYPE_CSTR, password);
    METAINFO_ADD_MEMBER(APIGetMobileSecretKeyMsg, FIELD_TYPE_CSTR, jsonStr);
}

DEFINE_METAINFO(APICeCmdCallbackMsg)
{
    METAINFO_CREATE(APICeCmdCallbackMsg);

    METAINFO_ADD_MEMBER(APICeCmdCallbackMsg, FIELD_TYPE_CSTR, esn);
    METAINFO_ADD_MEMBER(APICeCmdCallbackMsg, FIELD_TYPE_CSTR, timestamp);
    METAINFO_ADD_MEMBER(APICeCmdCallbackMsg, FIELD_TYPE_CSTR, jsonStr);
    METAINFO_ADD_MEMBER(APICeCmdCallbackMsg, FIELD_TYPE_CSTR, signature);
}

DEFINE_METAINFO(APICeEchoMsg)
{
    METAINFO_CREATE(APICeEchoMsg);

    METAINFO_ADD_MEMBER(APICeEchoMsg, FIELD_TYPE_CSTR, esn);
    METAINFO_ADD_MEMBER(APICeEchoMsg, FIELD_TYPE_CSTR, timestamp);
    METAINFO_ADD_MEMBER(APICeEchoMsg, FIELD_TYPE_CSTR, jsonStr);
    METAINFO_ADD_MEMBER(APICeEchoMsg, FIELD_TYPE_CSTR, signature);
}

DEFINE_METAINFO(APICeStartupMsg)
{
    METAINFO_CREATE(APICeStartupMsg);

    METAINFO_ADD_MEMBER(APICeStartupMsg, FIELD_TYPE_CSTR, esn);
    METAINFO_ADD_MEMBER(APICeStartupMsg, FIELD_TYPE_CSTR, timestamp);
    METAINFO_ADD_MEMBER(APICeStartupMsg, FIELD_TYPE_CSTR, jsonStr);
    METAINFO_ADD_MEMBER(APICeStartupMsg, FIELD_TYPE_CSTR, signature);
}

DEFINE_METAINFO(APIConnectVpeMsg)
{
    METAINFO_CREATE(APIConnectVpeMsg);

    METAINFO_ADD_MEMBER(APIConnectVpeMsg, FIELD_TYPE_CSTR, esn);
    METAINFO_ADD_MEMBER(APIConnectVpeMsg, FIELD_TYPE_CSTR, timestamp);
    METAINFO_ADD_MEMBER(APIConnectVpeMsg, FIELD_TYPE_CSTR, jsonStr);
    METAINFO_ADD_MEMBER(APIConnectVpeMsg, FIELD_TYPE_CSTR, signature);
    //METAINFO_ADD_MEMBER(APIConnectVpeMsg, FIELD_TYPE_CSTR, data);
    METAINFO_ADD_MEMBER(APIConnectVpeMsg, FIELD_TYPE_INT, timeout);
    //METAINFO_ADD_MEMBER(APIConnectVpeMsg, FIELD_TYPE_CSTR, osVersion);
}

char * assemble_msg(char *msg_name, char *api_msg){
    char str[5000];
    strcpy(str, msgheader);
    strcat(str, msg_name);
    strcat(str, "\":");
    strcat(str, api_msg);
    strcat(str, "}");

    //返回的字符串要free掉
    char* s = (char *)malloc( 5000 * sizeof(char) );
    strcpy(s, str);
    return s;
}
