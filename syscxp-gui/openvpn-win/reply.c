#include "reply.h"
#include "logger.h"


char replyheader[] = "com.syscxp.header.sdwan.ce.daemon.mobile.";
char api_err_reply[] = "com.syscxp.header.message.APIReply";

DEFINE_METAINFO(APICeCmdCallbackReply) {
    METAINFO_CREATE(APICeCmdCallbackReply);
    METAINFO_ADD_MEMBER(APICeCmdCallbackReply, FIELD_TYPE_CSTR, error);
    METAINFO_ADD_MEMBER(APICeCmdCallbackReply, FIELD_TYPE_CSTR, success);
}

DEFINE_METAINFO(APIGetMobileSecretKeyReply) {
    METAINFO_CREATE(APIGetMobileSecretKeyReply);
    METAINFO_ADD_MEMBER(APIGetMobileSecretKeyReply, FIELD_TYPE_CSTR, esn);
    METAINFO_ADD_MEMBER(APIGetMobileSecretKeyReply, FIELD_TYPE_CSTR, secretKey);
    METAINFO_ADD_MEMBER(APIGetMobileSecretKeyReply, FIELD_TYPE_CSTR, success);
}

DEFINE_METAINFO(APICeEchoReply) {
    METAINFO_CREATE(APICeEchoReply);
    METAINFO_ADD_MEMBER(APICeEchoReply, FIELD_TYPE_CSTR, success);
    METAINFO_ADD_MEMBER(APICeEchoReply, FIELD_TYPE_CSTR, esn);
    METAINFO_ADD_MEMBER(APICeEchoReply, FIELD_TYPE_CSTR, deleted);
    METAINFO_ADD_MEMBER(APICeEchoReply, FIELD_TYPE_CSTR, finalized);
    METAINFO_ADD_MEMBER(APICeEchoReply, FIELD_TYPE_CSTR, cmd);
}

DEFINE_METAINFO(APICeStartupReply) {
    METAINFO_CREATE(APICeStartupReply);
    METAINFO_ADD_MEMBER(APICeStartupReply, FIELD_TYPE_CSTR, success);
    METAINFO_ADD_MEMBER(APICeStartupReply, FIELD_TYPE_CSTR, esn);
    METAINFO_ADD_MEMBER(APICeStartupReply, FIELD_TYPE_CSTR, state);
    METAINFO_ADD_MEMBER(APICeStartupReply, FIELD_TYPE_CSTR, configStatus);
    METAINFO_ADD_MEMBER(APICeStartupReply, FIELD_TYPE_CSTR, vpeIps);
    METAINFO_ADD_MEMBER(APICeStartupReply, FIELD_TYPE_CSTR, masterLocalIp);
    METAINFO_ADD_MEMBER(APICeStartupReply, FIELD_TYPE_CSTR, slaveLocalIp);
    METAINFO_ADD_MEMBER(APICeStartupReply, FIELD_TYPE_CSTR, masterGateway);
    METAINFO_ADD_MEMBER(APICeStartupReply, FIELD_TYPE_CSTR, slaveGateway);
    METAINFO_ADD_MEMBER(APICeStartupReply, FIELD_TYPE_CSTR, tap);
    METAINFO_ADD_MEMBER(APICeStartupReply, FIELD_TYPE_CSTR, serviceCidrs);
    METAINFO_ADD_MEMBER(APICeStartupReply, FIELD_TYPE_CSTR, secretKey);
    METAINFO_ADD_MEMBER(APICeStartupReply, FIELD_TYPE_INT, bandwidth);
}

DEFINE_METAINFO(APIConnectVpeEvent) {
    METAINFO_CREATE(APIConnectVpeEvent);
    METAINFO_ADD_MEMBER(APIConnectVpeEvent, FIELD_TYPE_CSTR, success);
    METAINFO_ADD_MEMBER(APIConnectVpeEvent, FIELD_TYPE_CSTR, esn);
    METAINFO_ADD_MEMBER(APIConnectVpeEvent, FIELD_TYPE_CSTR, error);

}

DEFINE_METAINFO(Cmd) {
    METAINFO_CREATE(Cmd);
    METAINFO_ADD_MEMBER(Cmd, FIELD_TYPE_CSTR, uuid);
    METAINFO_ADD_MEMBER(Cmd, FIELD_TYPE_CSTR, cmd);
    METAINFO_ADD_MEMBER(Cmd, FIELD_TYPE_CSTR, cmdData);

}

DEFINE_METAINFO(CmdJson) {
    METAINFO_CREATE(CmdJson);
    METAINFO_ADD_MEMBER(CmdJson, FIELD_TYPE_CSTR, cmdUuid);
    METAINFO_ADD_MEMBER(CmdJson, FIELD_TYPE_CSTR, jsonData);
    METAINFO_ADD_MEMBER(CmdJson, FIELD_TYPE_BOOL, success);
    METAINFO_ADD_MEMBER(CmdJson, FIELD_TYPE_CSTR, errMsg);

}

char *unassemble_reply(char *reply_name, char *api_reply_body) {
    char str[5000];
    cJSON *reply;
    strcpy(str, replyheader);
    strcat(str, reply_name);

    // 返回的字符串要free掉
    char *s = (char *) malloc(8000 * sizeof(char));
    //strcpy(s, str);

    cJSON *json_reply = cJSON_Parse(api_reply_body);
    cJSON *item = cJSON_GetObjectItem(json_reply, "result");
    cJSON *reply_result = cJSON_Parse(item->valuestring);

    if (cJSON_GetObjectItem(reply_result, api_err_reply)) {
//        cJSON * item = cJSON_GetObjectItem(reply_result, api_err_reply);
//        cJSON *error = cJSON_GetObjectItem(item, "error");
//        char *details = cJSON_GetObjectItem(error, "details")->valuestring;
        // strcpy(s, "false");
        reply = cJSON_GetObjectItem(reply_result, api_err_reply);
    } else {
        reply = cJSON_GetObjectItem(reply_result, str);
    }

    strcpy(s, cJSON_Print(reply));
    return s;

}

int is_error_reply(char *reply){
    cJSON *REPLY= cJSON_Parse(reply);
    char * success=cJSON_Print(cJSON_GetObjectItem(REPLY, "success"));

    char is_error[10];

    strcpy(is_error, success);

    //write_log_file(success);

    if (strcmp(is_error,"false")==0){
        return 1;
    }
    return 0;
}
