#include "client.h"
extern options_t o;
void start_by_config(){
    //TODO 根据配置启动vpn

    cpe_connected=1;

}

/*void save_secret_key(char *secretKey){
    secret_key=secretKey;
    FILE *fp = NULL;

    fp = fopen(SECRET_KEY_FILE, "w+");

    fputs( secretKey, fp);
    fclose(fp);

    write_log_file(secretKey);
}

char * get_secret_key(){
    char* s = (char *)malloc( 50 * sizeof(char) );
    if(secret_key!=NULL){

    }else{
        read_local_secret_key();
    }
    strcpy(s, secret_key);
    return s;
}

void read_local_secret_key(){
    FILE *fp = NULL;
    char buff[255];

    fp = fopen(SECRET_KEY_FILE, "r");

    if(fp!=NULL){
        fgets(buff, 255, (FILE*)fp);
        secret_key=buff;
        fclose(fp);
    }else{
        secret_key="nokey";
    }
}*/

char * get_serve_url(){
    char str[100];
    strcpy(str, CTRL_PROTOCOL);
    strcat(str, "://");
    strcat(str, CTRL_DOMAIN_NAME);

    char* s=(char *)malloc( 100 * sizeof(char) );
    strcpy(s, str);
    return s;
}

char* ReplaceSubStr(const char* str, const char* srcSubStr, const char* dstSubStr, char* out)
{
    if (!str&&!out)
    {
        return NULL;
    }
    if (!srcSubStr && !dstSubStr)
    {
        return out;
    }
    char *out_temp = out;
    int src_len = strlen(srcSubStr);
    int dst_len = strlen(dstSubStr);
    while (*str!='\0')
    {
        if (*str == *srcSubStr)
        {
            //可能发生替换
            const char* str_temp = str;
            int flag = 0;
            for (int i = 0; i < src_len; i++)
            {
                if (str_temp[i]!=srcSubStr[i])
                {
                    flag = 1;
                    break;
                }
            }
            if (flag)
            {
                //说明不用替换
                *out_temp++ = *str++;
            }
            else
            {
                //说明要替换
                for (int i = 0; i < dst_len; i++)
                {
                    *out_temp++ = dstSubStr[i];
                }
                str = str + src_len;
            }
        }
        else
        {
            //不用替换
            *out_temp++ = *str++;
        }
    }
    *out_temp = 0;
    return out;
}

char * enable_config(cJSON *cmdData){

    write_log_file("enable_config start");

    cJSON *data;

    data=cJSON_GetObjectItem(cmdData, "tap");
    strcpy(tap, data->valuestring);

    data=cJSON_GetObjectItem(cmdData, "masterRemoteIp");
    _tcscpy(master_remote_ip, data->valuestring);

    data=cJSON_GetObjectItem(cmdData, "masterRemotePort");
    _tcscpy(master_remote_port, cJSON_Print(data));

    data=cJSON_GetObjectItem(cmdData, "slaveRemoteIp");
    _tcscpy(slave_remote_ip, data->valuestring);

    data=cJSON_GetObjectItem(cmdData, "slaveRemotePort");
    _tcscpy(slave_remote_port, cJSON_Print(data));

    data=cJSON_GetObjectItem(cmdData, "cipher");
    _tcscpy(syscxp_cipher, data->valuestring);

    TCHAR filename[2 * MAX_PATH];

    /*TCHAR cafile[]=TEXT("ca.crt");
    TCHAR certfile[]=TEXT("client.crt");
    TCHAR keyfile[]=TEXT("client.key");*/

    FILE* fp = NULL;

    data=cJSON_GetObjectItem(cmdData, "ca");

    ReplaceSubStr(data->valuestring,"\n","\\n",syscxp_ca);
    //_tcscpy(syscxp_ca, );

    //write_log_file(syscxp_ca);

    _sntprintf_0(filename, _T("%s\\%s\\%s"), o.config_dir, o.conn[config].config_name, _T("ca.crt"));
    if ((fp = _tfopen(filename, TEXT("w+"))) != NULL){
        fputs(data->valuestring, fp);
    }
    fclose(fp);

    data=cJSON_GetObjectItem(cmdData, "cert");

    ReplaceSubStr(data->valuestring,"\n","\\n",syscxp_cert);
    //_tcscpy(syscxp_cert, data->valuestring);

    _sntprintf_0(filename, _T("%s\\%s\\%s"), o.config_dir, o.conn[config].config_name, _T("client.crt"));
    if ((fp = _tfopen(filename, TEXT("w+"))) != NULL){
        fputs(data->valuestring, fp);
    }
    fclose(fp);

    data=cJSON_GetObjectItem(cmdData, "key");

    ReplaceSubStr(data->valuestring,"\n","\\n",syscxp_key);
    //_tcscpy(syscxp_key, data->valuestring);

    _sntprintf_0(filename, _T("%s\\%s\\%s"), o.config_dir, o.conn[config].config_name, _T("client.key"));
    if ((fp = _tfopen(filename, TEXT("w+"))) != NULL){
        fputs(data->valuestring, fp);
    }
    fclose(fp);


   /*TCHAR filename[2 * MAX_PATH];
   FILE* fp = NULL;

   _sntprintf_0(filename, _T("%s\\%s\\%s"), o.config_dir, o.conn[config].config_name, o.conn[config].config_file);

   write_log_file(filename);
   if ((fp = _tfopen(filename, TEXT("w+"))) != NULL)
   {
       cJSON *data;

       fputs("client\n", fp);

       fputs("dev tap\n", fp);

       fputs("proto udp\n", fp);
       fputs("remote ", fp);

       data=cJSON_GetObjectItem(cmdData, "masterRemoteIp");

       fputs(data->valuestring, fp);
       fputs("  ", fp);
       data=cJSON_GetObjectItem(cmdData, "masterRemotePort");

       fputs(cJSON_Print(data), fp);
       fputs("\n", fp);

       fputs("remote ", fp);

       data=cJSON_GetObjectItem(cmdData, "slaveRemoteIp");

       fputs(data->valuestring, fp);
       fputs("  ", fp);
       data=cJSON_GetObjectItem(cmdData, "slaveRemotePort");

       fputs(cJSON_Print(data), fp);
       fputs("\n", fp);

       fputs("ping 5\n", fp);
       fputs("comp-lzo\n", fp);
       fputs("nobind\n", fp);
       fputs("cipher ", fp);
       data=cJSON_GetObjectItem(cmdData, "cipher");

       fputs(data->valuestring, fp);
       fputs("\n", fp);

       fputs("verb 4\n", fp);
       fputs("mute 20\n", fp);
       fputs("auth-user-pass\n", fp);
       fputs(";log\n", fp);
       fputs(";log-append\n", fp);
       fputs("proto udp\n", fp);
       fputs("<ca>\n", fp);
       data=cJSON_GetObjectItem(cmdData, "ca");

       fputs(data->valuestring, fp);

       fputs("</ca>\n", fp);
       fputs("<cert>\n", fp);
       data=cJSON_GetObjectItem(cmdData, "cert");

       fputs(data->valuestring, fp);

       fputs("</cert>\n", fp);
       fputs("<key>\n", fp);
       data=cJSON_GetObjectItem(cmdData, "key");

       fputs(data->valuestring, fp);

       fputs("</key>\n", fp);

   }
   fclose(fp);*/


    write_log_file("enable_config end");
}

char * enable_init(cJSON *cmdData){
    //TODO
}

char * disable(){
    //TODO
}

char * init(cJSON *cmdData){
    //TODO
}

char * report_wan_info(cJSON *cmdData){
    //TODO
}

char * route_add(cJSON *cmdData){
    write_log_file("add route start============>>>");
    cJSON *root;
    root= cJSON_GetObjectItem(cmdData, "cidrs");
    int array_size = cJSON_GetArraySize(root);
    int i = 0;
    cJSON *item;
    HINSTANCE status;
    for(i=0; i< array_size; i++) {
        item = cJSON_GetArrayItem(root, i);

        char cmdline[200];

        int i, config = 0;
        for (i = 0; i < o.num_configs; i++) {
            if (o.conn[i].state == connected) {
                config = i;
            }
        }

        sprintf(cmdline, "%s %s %ws", "cmd.exe /c route add", item->valuestring, &o.conn[config].ip);

        write_log_file("cmdline======>>>>>>");
        write_log_file(cmdline);

        //status=ShellExecuteW(NULL, L"open", L"cmd.exe", cmdline , NULL, SW_HIDE);

        status=WinExec(cmdline, SW_HIDE);

        //system(cmdline);

        if (status > (HINSTANCE) 32) /* Success */
        {   char msg[50];
            sprintf(msg, "%s add success",item->valuestring);
            write_log_file(msg);
        }
        else
        {
            char msg[50];
            sprintf(msg, "%s add failed",item->valuestring);
            write_log_file(msg);
        }

    }
    write_log_file("add route end============>>>");
}

char * route_delete(cJSON *cmdData){
    write_log_file("delete route start============>>>");
    cJSON *root;
    root= cJSON_GetObjectItem(cmdData, "cidrs");
    int array_size = cJSON_GetArraySize(root);
    int i = 0;
    cJSON *item;
    HINSTANCE status;
    for(i=0; i< array_size; i++) {
        item = cJSON_GetArrayItem(root, i);

        char cmdline[100];

        sprintf(cmdline, "%s %s", "cmd.exe /c route delete", item->valuestring);

        write_log_file("cmdline======>>>>>>");
        write_log_file(cmdline);

        //status=ShellExecuteW(NULL, NULL, L"cmd.exe", cmdline , NULL, SW_HIDE);

        status=WinExec(cmdline, SW_HIDE);


        if (status > (HINSTANCE) 32) /* Success */
        {   char msg[50];
            sprintf(msg, "%s delete success",item->valuestring);
            write_log_file(msg);
        }
        else
        {
            char msg[50];
            sprintf(msg, "%s delete failed",item->valuestring);
            write_log_file(msg);
        }
    }
    write_log_file("delete route end============>>>");
}

char * update_bandwidth(cJSON *cmdData){
    //TODO
}

char * upgrade_start(cJSON *cmdData){
    //TODO
}

void restart(){
    //TODO
}