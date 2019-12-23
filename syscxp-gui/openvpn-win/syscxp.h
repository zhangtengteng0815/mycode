#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include "windows.h"
#include "http.h"
#include "client.h"
#include "message.h"
#include "reply.h"
#include "ping.h"
#include "logger.h"
#include "MD5.h"
#include "cJSON/cJSON.h"

typedef struct The_user   //syscxp用户
{
    TCHAR id[11];
    TCHAR pwd[20];
}syscxpuser;


int save_key();
void ce_startup();
void connect_to_vpe();
int ce_echo();
char ** cmd_handler(struct Cmd* cmd);
void cmd_callback(char * cmdUuid,char ** result);
DWORD WINAPI call_server(LPVOID lpParam);
char * call(char * postdate);
char * sync_polling(ft_http_client_t *http,char * uuid);
char * signature(int num,...);
