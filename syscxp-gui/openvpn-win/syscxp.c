#include "syscxp.h"

int save_key(){
    write_log_file("save key function");
    REGISTER_METAINFO(APIGetMobileSecretKeyMsg);
    struct APIGetMobileSecretKeyMsg *msg = OBJECT_NEW(APIGetMobileSecretKeyMsg);

    char esnstr[50];
    sprintf(esnstr, "%ws", esn );
    msg->esn = CS(esnstr);

    int t=(int)time(NULL);
    char time_stamp[15];
    sprintf(time_stamp, "%d", t);
    strcat(time_stamp, "000");
    msg->timestamp = CS(time_stamp);


    char passstr[50];
    sprintf(passstr, "%ws", secretPassword );
    msg->password = CS(passstr);
    //msg->jsonStr = CS("");

    CSTR json = OBJECT_TO_JSON(msg, APIGetMobileSecretKeyMsg);
    char msgname[] = "APIGetMobileSecretKeyMsg";
    char * msgres=assemble_msg(&msgname, json.cstr);
    char * body=call(msgres);

    write_log_file(msgres);
    //write_log_file(body);

    char * reply_name = "APIGetMobileSecretKeyReply";
    char *reply = unassemble_reply(reply_name, body);

    write_log_file(reply);

    if (is_error_reply(reply)){
        write_log_file("GetMobileSecretKey failed");
        if (msgres){
            free(msgres);
            if(body){
                free(body);
            }
            if (reply){
                free(reply);
            }
        }
        return 0;
    }

    REGISTER_METAINFO(APIGetMobileSecretKeyReply);
    CSTR reply_json = CS(reply);
    struct APIGetMobileSecretKeyReply *api_reply = OBJECT_NEW(APIGetMobileSecretKeyReply);
    api_reply = OBJECT_FROM_JSON(APIGetMobileSecretKeyReply, reply_json.cstr);

    strcpy(secret_key, api_reply->secretKey.cstr);
    //secret_key=api_reply->secretKey.cstr;

    write_log_file("secret_key ===>");
    write_log_file(secret_key);

    free(msgres);
    free(body);
    free(reply);

    return 1;
}

void ce_startup(){
    write_log_file("ce_startup  FUNCTION");

    REGISTER_METAINFO(APICeStartupMsg);
    struct APICeStartupMsg *msg = OBJECT_NEW(APICeStartupMsg);

    char esnstr[50];
    sprintf(esnstr, "%ws", esn );
    msg->esn = CS(esnstr);

    int t=(int)time(NULL);
    char time_stamp[15];
    sprintf(time_stamp, "%d", t);
    strcat(time_stamp, "000");
    msg->timestamp = CS(time_stamp);
    //msg->timestamp = (int)time(NULL);

    msg->jsonStr = CS("");

    char * sign=signature(3,msg->esn.cstr,msg->jsonStr.cstr,msg->timestamp.cstr);

    write_log_file(sign);

    msg->signature = CS(sign);
    CSTR json = OBJECT_TO_JSON(msg, APICeStartupMsg);
    char msgname[] = "APICeStartupMsg";
    char * msgres=assemble_msg(&msgname, json.cstr);

    write_log_file(msgres);

    char * body=call(msgres);

    //write_log_file(body);

    char * reply_name = "APICeStartupReply";
    char *reply = unassemble_reply(reply_name, body);

    write_log_file(reply);

    if (is_error_reply(reply)){
        free(sign);
        free(msgres);
        free(body);
        free(reply);
        return;
    }

    /*
    REGISTER_METAINFO(APICeStartupReply);
    CSTR reply_json = CS(reply);
    struct APICeStartupReply *api_reply = OBJECT_NEW(APICeStartupReply);
    api_reply = OBJECT_FROM_JSON(APICeStartupReply, reply_json.cstr);
     */

    cJSON *statreply= cJSON_Parse(reply);
    /*
    cJSON *val;
    val = cJSON_GetObjectItem(statreply, "esn");
    write_log_file(val->valuestring);
    val = cJSON_GetObjectItem(statreply, "state");
    write_log_file(val->valuestring);
    val = cJSON_GetObjectItem(statreply, "configStatus");
    write_log_file(val->valuestring);
    val = cJSON_GetObjectItem(statreply, "vpeIps");
    int vps_size=cJSON_GetArraySize(val);
    int n = 0;
    cJSON *vps;
    for(n=0; n< vps_size; n++) {
        vps = cJSON_GetArrayItem(val, n);
        write_log_file(vps->valuestring);
    }
    val = cJSON_GetObjectItem(statreply, "bandwidth");
    char bandwith[20];
    sprintf(bandwith,"%d",val->valueint);
    write_log_file(bandwith);
    val = cJSON_GetObjectItem(statreply, "success");
    write_log_file(cJSON_Print(val));
     */


    /*write_log_file(api_reply->success.cstr);
    write_log_file(api_reply->state.cstr);
    write_log_file(api_reply->configStatus.cstr);
    char tt[20];
    sprintf(tt,"%d",api_reply->bandwidth);
    write_log_file(tt);
    write_log_file(api_reply->vpeIps.cstr);
    write_log_file(api_reply->esn.cstr);
    write_log_file("1111");*/

    char * success=cJSON_Print(cJSON_GetObjectItem(statreply, "success"));

    //write_log_file(success);

    if(strcmp(success,"true")==0){
        cpe_started = 1;

        strcpy(configStatus, cJSON_GetObjectItem(statreply, "configStatus")->valuestring);

        if(strcmp(cJSON_GetObjectItem(statreply, "configStatus")->valuestring, "Config") == 0 && strcmp(cJSON_GetObjectItem(statreply, "state")->valuestring, "Enabled") == 0){
            cJSON *root;
            root=cJSON_GetObjectItem(statreply, "masterGateway");
            strcpy(master_gateway, root->valuestring);

            //write_log_file(master_gateway);

            root=cJSON_GetObjectItem(statreply, "slaveGateway");
            strcpy(slave_gateway, root->valuestring);

            //write_log_file(slave_gateway);

            //master_gateway = cJSON_GetObjectItem(statreply, "masterGateway")->valuestring;
            //slave_gateway = cJSON_GetObjectItem(statreply, "slaveGateway")->valuestring;

            root= cJSON_GetObjectItem(statreply, "serviceCidrs");
            int array_size = cJSON_GetArraySize(root);
            int i = 0;
            cJSON *item;
            for(i=0; i< array_size; i++) {
                item = cJSON_GetArrayItem(root, i);
                //service_cidrs[i]=item->valuestring;
                strcpy(service_cidrs[i], item->valuestring);

                //write_log_file(service_cidrs[i]);
            }

            //service_cidrs = api_reply->serviceCidrs.cstr;

            root=cJSON_GetObjectItem(statreply, "tap");
            strcpy(tap, root->valuestring);

            //write_log_file(tap);

            //tap= cJSON_GetObjectItem(statreply, "tap")->valuestring;
            //tap = api_reply->tap.cstr;
            root=cJSON_GetObjectItem(statreply, "masterLocalIp");
            strcpy(master_local_ip, root->valuestring);

            //write_log_file(master_local_ip);

            root=cJSON_GetObjectItem(statreply, "slaveLocalIp");
            strcpy(slave_local_ip, root->valuestring);

            //write_log_file(slave_local_ip);

            //master_local_ip = cJSON_GetObjectItem(statreply, "masterLocalIp")->valuestring;
            //slave_local_ip = cJSON_GetObjectItem(statreply, "slaveLocalIp")->valuestring;


            root=cJSON_GetObjectItem(statreply, "masterRemoteIp");
            _tcscpy(master_remote_ip, root->valuestring);

            root=cJSON_GetObjectItem(statreply, "masterRemotePort");
            _tcscpy(master_remote_port, cJSON_Print(root));

            root=cJSON_GetObjectItem(statreply, "slaveRemoteIp");
            _tcscpy(slave_remote_ip, root->valuestring);

            root=cJSON_GetObjectItem(statreply, "slaveRemotePort");
            _tcscpy(slave_remote_port, cJSON_Print(root));

            root=cJSON_GetObjectItem(statreply, "cipher");
            _tcscpy(syscxp_cipher, root->valuestring);


            root=cJSON_GetObjectItem(statreply, "ca");
            ReplaceSubStr(root->valuestring,"\n","\\n",syscxp_ca);

            root=cJSON_GetObjectItem(statreply, "cert");
            ReplaceSubStr(root->valuestring,"\n","\\n",syscxp_cert);

            root=cJSON_GetObjectItem(statreply, "key");
            ReplaceSubStr(root->valuestring,"\n","\\n",syscxp_key);


            start_by_config();

        }else{
            cJSON *root= cJSON_GetObjectItem(statreply, "vpeIps");
            int array_size = cJSON_GetArraySize(root);
            int i = 0;
            cJSON *item;
            for(i=0; i< array_size; i++) {
                item = cJSON_GetArrayItem(root, i);
                strcpy(vpe_ips[i], item->valuestring);
                //vpe_ips[i]=item->valuestring;
                //write_log_file(vpe_ips[i]);
            }
            /*if(root){
                cJSON_Delete(root);
            }*/
            //vpe_ips=api_reply->vpeIps.cstr;
        }


        cJSON *secretKey=cJSON_GetObjectItem(statreply, "secretKey");

        if (secretKey){

            //write_log_file(secret_key);

            strcpy(secret_key, secretKey->valuestring);
            //secret_key=secretKey->valuestring;
            //save_secret_key(api_reply->secretKey.cstr);
        }
        write_log_file(secret_key);

    }
    if (sign){
        free(sign);
    }
    if (msgres){
        free(msgres);
    }
    if (body){
        free(body);
    }
    if (reply){
        free(reply);
    }


    if(statreply){
        cJSON_Delete(statreply);
    }

    write_log_file("ce_startup function end");

}

void connect_to_vpe() {
    if (cpe_deleted || cpe_connected) {
        return;
    }
    if (vpe_ips == NULL) {
        return;
    }

    char *out = do_ping();

    //write_log_file(out);

    REGISTER_METAINFO(APIConnectVpeMsg);
    struct APIConnectVpeMsg *msg = OBJECT_NEW(APIConnectVpeMsg);

    char esnstr[50];
    sprintf(esnstr, "%ws", esn );
    msg->esn = CS(esnstr);

    int t=(int)time(NULL);
    char time_stamp[15];
    sprintf(time_stamp, "%d", t);
    strcat(time_stamp, "000");
    msg->timestamp = CS(time_stamp);
    //msg->timestamp = (int)time(NULL);



/*    char out[1000] = "{\"ipPacketStructs\":";
    strcat( out, pingResult);*/


    msg->jsonStr = CS(out);


    msg->timeout = 180;

    //msg->osVersion = CS("123");
    //msg->data = CS("123");

    char * sign=signature(3,msg->esn.cstr,msg->jsonStr.cstr,msg->timestamp.cstr);

    //write_log_file(sign);

    msg->signature = CS(sign);
    CSTR json = OBJECT_TO_JSON(msg, APIConnectVpeMsg);
    char msgname[] = "APIConnectVpeMsg";
    char * msgres=assemble_msg(&msgname, json.cstr);

    write_log_file(msgres);

    char * body=call(msgres);

    //write_log_file(body);

    /*if (pingResult) {
        free(pingResult);
    }*/

    char * reply_name = "APIConnectVpeEvent";
    char *reply = unassemble_reply(reply_name, body);

    write_log_file(reply);

    if (is_error_reply(reply)){
        //free(out);
        //free(sign);
        //free(msgres);
        free(body);
        free(reply);

        return;
    }


    cJSON *connectReply= cJSON_Parse(reply);
    char * success=cJSON_Print(cJSON_GetObjectItem(connectReply, "success"));
    //write_log_file(success);

    /*REGISTER_METAINFO(APIConnectVpeEvent);
    CSTR reply_json = CS(reply);
    struct APIConnectVpeEvent *api_reply = OBJECT_NEW(APIConnectVpeEvent);
    api_reply = OBJECT_FROM_JSON(APIConnectVpeEvent, reply_json.cstr);*/

    if (strcmp(success,"true")==0){
        write_log_file("Connect VPE Successed");
        cpe_connected = 1;
    }else{
        write_log_file("Connect VPE Failed");
        cpe_connected = 0;
    }


    //write_log_file(sign);
    //write_log_file(msgres);
    //write_log_file(body);
    //write_log_file(reply);

    /*if (sign){
        free(sign);
    }*/
   /*
    if (msgres){
        free(msgres);
    }*/
   /* if (out){
        free(out);
    }*/
    if (body){
        free(body);
    }
    if (reply){
        free(reply);
    }
    if(connectReply){
        cJSON_Delete(connectReply);
    }

}

int ce_echo(){
    REGISTER_METAINFO(APICeEchoMsg);
    struct APICeEchoMsg *msg = OBJECT_NEW(APICeEchoMsg);

    char esnstr[50];
    sprintf(esnstr, "%ws", esn );
    msg->esn = CS(esnstr);


    int t=(int)time(NULL);
    char time_stamp[15];
    sprintf(time_stamp, "%d", t);
    strcat(time_stamp, "000");
    msg->timestamp = CS(time_stamp);
    //msg->timestamp = (int)time(NULL);


    msg->jsonStr = CS("");
    char * sign=signature(3,msg->esn.cstr,msg->jsonStr.cstr,msg->timestamp.cstr);
    msg->signature = CS(sign);

    CSTR json = OBJECT_TO_JSON(msg, APICeEchoMsg);
    char msgname[] = "APICeEchoMsg";
    char * msgres=assemble_msg(&msgname, json.cstr);

    write_log_file(msgres);

    char * body=call(msgres);

    //write_log_file(body);

    char * reply_name = "APICeEchoReply";
    char *reply = unassemble_reply(reply_name, body);

    write_log_file(reply);

    if (is_error_reply(reply)){
        free(sign);
        free(msgres);
        free(body);
        free(reply);
        return 0;
    }

    cJSON *echoReply= cJSON_Parse(reply);
    char * success=cJSON_Print(cJSON_GetObjectItem(echoReply, "success"));


    /*REGISTER_METAINFO(APICeEchoReply);
    CSTR reply_json = CS(reply);
    struct APICeEchoReply *api_reply = OBJECT_NEW(APICeEchoReply);
    api_reply = OBJECT_FROM_JSON(APICeEchoReply, reply_json.cstr);*/

    free(sign);
    free(msgres);
    free(body);
    free(reply);

    if(strcmp(success,"true")==0){
        char * deleted=cJSON_Print(cJSON_GetObjectItem(echoReply, "deleted"));
        if(strcmp(deleted,"true")==0){
            cpe_deleted=1;
        } else{
            cpe_deleted=0;
        }

        char * cmdstr=cJSON_Print(cJSON_GetObjectItem(echoReply, "cmd"));

        if (cmdstr!=NULL){
            write_log_file("deal cmd");

            REGISTER_METAINFO(Cmd);
            CSTR cmd_json = CS(cmdstr);
            struct Cmd * cmd = OBJECT_NEW(Cmd);
            cmd = OBJECT_FROM_JSON(Cmd, cmd_json.cstr);

            char *result=cmd_handler(cmd);
            //TODO 测试启动
            cmd_callback(cmd->uuid.cstr, result);

            return 1;
        }
    }

    return 0;
}

char ** cmd_handler(struct Cmd* cmd)
{
    //result第一个值放执行是否成功true false
    //第二个值放执行返回值
    static char * result[2];
    cJSON *cmdData=cJSON_Parse(cmd->cmdData.cstr);;
    //TODO 执行cmd
    if(strcmp(cmd->cmd.cstr,"StartConfigCeCmd")==0){
        /*write_log_file("1111111111111111111111");
        cJSON *data=cJSON_GetObjectItem(cmdData, "vpnType");
        write_log_file(data->valuestring);
        data=cJSON_GetObjectItem(cmdData, "ca");
        write_log_file(data->valuestring);*/

        result[1]=enable_config(cmdData);
        result[0]="true";
        get_Config_Ce_Cmd=1;

    }else if(strcmp(cmd->cmd.cstr,"StartInitCeCmd")==0){
        result[1]=enable_init(cmdData);
        result[0]="true";
    }else if(strcmp(cmd->cmd.cstr,"StopCeCmd")==0){
        result[1]=disable();
        result[0]="true";
    }else if(strcmp(cmd->cmd.cstr,"InitCmd")==0){
        result[1]=init(cmdData);
        result[0]="true";
    }else if(strcmp(cmd->cmd.cstr,"ReportWanInfoCmd")==0){
        result[1]=report_wan_info(cmdData);
        result[0]="true";
    }else if(strcmp(cmd->cmd.cstr,"AddRouteCmd")==0){
        result[1]=route_add(cmdData);
        result[0]="true";
    }else if(strcmp(cmd->cmd.cstr,"DeleteRouteCmd")==0){
        result[1]=route_delete(cmdData);
        result[0]="true";
    }else if(strcmp(cmd->cmd.cstr,"UpdateBandwidthCmd")==0){
        result[1]=update_bandwidth(cmdData);
        result[0]="true";
    }else if(strcmp(cmd->cmd.cstr,"UpgradeCmd")==0){
        result[1]=upgrade_start(cmdData);
        result[0]="true";
        cmd_callback(cmd->uuid.cstr,result);
        restart();
        return ;
    }else{
        write_log_file("unknown cmd ==>");
        write_log_file(cmd->cmd.cstr);
    }
    cJSON_Delete(cmdData);
    return result;
}

void cmd_callback(char * cmdUuid,char ** result){
    REGISTER_METAINFO(APICeCmdCallbackMsg);
    struct APICeCmdCallbackMsg *msg = OBJECT_NEW(APICeCmdCallbackMsg);

    char esnstr[50];
    sprintf(esnstr, "%ws", esn );
    msg->esn = CS(esnstr);

    int t=(int)time(NULL);
    char time_stamp[15];
    sprintf(time_stamp, "%d", t);
    strcat(time_stamp, "000");
    msg->timestamp = CS(time_stamp);
    //msg->timestamp = (int)time(NULL);

    REGISTER_METAINFO(CmdJson);
    struct CmdJson *cmdobj = OBJECT_NEW(CmdJson);
    cmdobj->cmdUuid=CS(cmdUuid);

    if (strcmp(result[0],"true")==0){
        cmdobj->success=1;
        cmdobj->jsonData=CS(result[1]);
    }else{
        cmdobj->success=0;
        cmdobj->errMsg=CS(result[1]);
    }
    CSTR cmdjson=OBJECT_TO_JSON(cmdobj, CmdJson);

    msg->jsonStr=CS(cmdjson.cstr);

    char * sign=signature(3,msg->esn.cstr,msg->jsonStr.cstr,msg->timestamp.cstr);
    msg->signature = CS(sign);

    CSTR json = OBJECT_TO_JSON(msg, APICeCmdCallbackMsg);
    char msgname[] = "APICeCmdCallbackMsg";
    char * msgres=assemble_msg(&msgname, json.cstr);

    //write_log_file(msgres);

    char * body=call(msgres);

    //write_log_file(body);

    free(sign);
    free(msgres);
    free(body);
};

char * call(char * postdate){
    ft_http_client_t* http=0;
    ft_http_init();
    http = ft_http_new();
    ft_http_set_timeout(http, 20000);

    char * ser_url=get_serve_url();

    //write_log_file(ser_url);

    char* body;

    while(1){
        body = ft_http_sync_post_request(http, ser_url,postdate,M_POST);
        //write_log_file(body);
        //if( http->error_code != ERR_OK){
        if( is_http_error(http) ){
            Sleep(1000);
        }else{
            break;
        }
    }

    //write_log_file(body);

    cJSON *root = cJSON_Parse(body);
    char* s = (char *)malloc( 8000 * sizeof(char) );

    if(root==NULL){
        strcpy(s, body);
    } else{
        cJSON *state = cJSON_GetObjectItem(root, "state");
        if ( state==NULL || strcmp(state->valuestring,"Done")==0){
            strcpy(s, body);
            //write_log_file(body);
        }else if(strcmp(state->valuestring,"Processing")==0){
            char *uuid = cJSON_GetObjectItem(root, "uuid")->valuestring;
            //s=sync_polling(http,uuid->valuestring);

            char url[200];
            strcpy(url,ser_url);
            strcat(url, "/result/");
            strcat(url, uuid);

            //write_log_file(url);

            int count =0;
            int timeout=120;

            while (count<120){
                //strcpy(body, ft_http_sync_request(http, url,M_GET));

                http=0;
                ft_http_init();
                http = ft_http_new();

                char *str= ft_http_sync_request(http, url,M_GET);
                Sleep(1000);
                //write_log_file(str);

                root = cJSON_Parse(str);

                if (root==NULL){
                    count++;
                    Sleep(1000);
                } else{
                    state = cJSON_GetObjectItem(root, "state");

                    if ( state !=NULL && strcmp(state->valuestring,"Done")==0){
                        strcpy(s, str);
                        break ;
                    }else{
                        Sleep(1000);
                    }
                    count++;
                }
            }
        }
    }

    while (ser_url)
    {
        free(ser_url);
        ser_url = NULL;
    }

    if (http) {
        ft_http_destroy(http);
    }
    ft_http_deinit();

    if(root){
        cJSON_Delete(root);
    }

    //write_log_file(s);

    return s;
}

char * sync_polling(ft_http_client_t * http,char * uuid){
    char * ser_url=get_serve_url();

    char url[30];
    strcpy(url,ser_url);
    strcat(url, "/result/");
    strcat(url, uuid);

    //write_log_file(url);

    int count =0;
    int timeout=120;
    char *s = (char *)malloc( 8000 * sizeof(char) );

    while (count<120){
        //strcpy(body, ft_http_sync_request(http, url,M_GET));
        char *body= ft_http_sync_request(http, url,M_GET);
        //write_log_file("body ===>>>");
        //write_log_file(body);

        cJSON *root = cJSON_Parse(body);

        cJSON *state = cJSON_GetObjectItem(root, "state");
        if (strcmp(state->valuestring,"Done")==0){
            strcpy(s, body);
            break ;
        }else{
            Sleep(1000);
        }
        count++;
    }

    while (ser_url)
    {
        free(ser_url);
        ser_url = NULL;
    }
    //write_log_file("sync_polling ======>>>>>");
    //write_log_file(s);
    return s;
}


char * signature(int num,...){
    va_list valist;
    /* 为 num 个参数初始化 valist */
    va_start(valist, num);

    char str[500];

    //char * secretKey=get_secret_key();

    /* 访问所有赋给 valist 的参数 */
    if (num==2){
        //esn
        strcpy( str, va_arg(valist, char*));

        strcat( str, ":");
        //timestamp
        strcat( str, va_arg(valist, char*));
        strcat( str, ":");
        //secret_key
        if(secret_key){
            strcat( str, secret_key);
        }

    }else if(num==3){
        //esn
        strcpy( str, va_arg(valist, char*));
        strcat( str, ":");
        //jsonStr
        strcat( str, va_arg(valist, char*));
        strcat( str, ":");

        //timestamp
        strcat( str, va_arg(valist, char*));
        strcat( str, ":");
        //secret_key
        if(secret_key){
            strcat( str, secret_key);
        }
    }else{
        write_log_file("signature 出错！");
    }
    //write_log_file("signature string -->");
    //write_log_file(str);

    //char msg[]="zhangsan";
    uint8_t result[500];
    md5((uint8_t*)str, strlen(str), result);
    char md5str[32];
    int j=0;
    int i;
    for (i = 0; i < 16; i++){
        j+=sprintf(md5str+j, "%2.2x", result[i]);
    }

    /* 清理为 valist 保留的内存 */
    va_end(valist);

    char* s = (char *)malloc( 32 * sizeof(char) );
    strcpy(s, md5str);

    //write_log_file("signature === >>>>");
    //write_log_file(s);

    return s;
}

DWORD WINAPI call_server(LPVOID lpParam)
{
    char * post_data =(char *) lpParam;
    write_log_file(post_data);
    ft_http_client_t* http=0;
    ft_http_init();
    http = ft_http_new();
    //ft_http_set_timeout(http, 4);

    char * ser_url=get_serve_url();
    //write_log_file(ser_url);
    //char * ser_url="http://192.168.211.73:8089/sdwan/api";
    const char* body = ft_http_sync_post_request(http, ser_url,post_data,M_POST);
    //free(ser_url);
    //write_log_file(body);
    while (ser_url)
    {
        free(ser_url);
        ser_url = NULL;
    }

    if (http) {
        ft_http_destroy(http);
    }
    ft_http_deinit();
}


