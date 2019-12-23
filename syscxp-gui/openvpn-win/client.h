#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "metadata.h"
#include "logger.h"
#include "main.h"
#include "options.h"
#include "cJSON/cJSON.h"

WCHAR esn[128];
WCHAR secretPassword[128];


//char * secret_key;
char secret_key[35];


int cpe_started;
int cpe_deleted;
int cpe_connected;

char tap[10];
char master_local_ip[20];
char slave_local_ip[20];

char master_gateway[20];
char slave_gateway[20];

char configStatus[20];
int get_Config_Ce_Cmd;

TCHAR master_remote_ip[20];
TCHAR master_remote_port[10];
TCHAR slave_remote_ip[20];
TCHAR slave_remote_port[10];
TCHAR syscxp_cipher[20];

char syscxp_ca[2000];
char syscxp_cert[5000];
char syscxp_key[2000];


char service_cidrs[10][20];

char vpn_type[20];

char vpe_ips[10][20];

char * get_serve_url();
void start_by_config();

//void save_secret_key(char *secretKey);
//char * get_secret_key();
//void read_local_secret_key();

char * disable();   // 断开vpn连接，登出
char * update_bandwidth(cJSON *cmdData);

char * report_wan_info(cJSON *cmdData); // 待确认

char * route_add(cJSON *cmdData);   // 待确认
char * route_delete(cJSON *cmdData);    // 待确认

char * upgrade_start(cJSON *cmdData);   // 待确认
void restart(); // 待确认

char * enable_config(cJSON *cmdData);  // 不需要
char * enable_init(cJSON *cmdData); // 不需要
char * init(cJSON *cmdData);    // 不需要

