#include "ping.h"

int ipPingCount = 0;
struct cJSON *pingIpsResults[10];

//计算校验和
u_short checksum(u_short *buffer, int len) {
    register int nleft = len;
    register u_short *w = buffer;
    register u_short answer;
    register int sum = 0;
    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1) {
        u_short u = 0;
        *(u_char * )(&u) = *(u_char *) w;
        sum += u;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return (answer);
}

//发送回应请求函数
int SendEchoRequest(SOCKET s, struct sockaddr_in *lpstToAddr) {
    static ECHOREQUEST echoReq;
    static int nId = 1;
    static int nSeq = 1;
    int nRet;
    echoReq.icmpHdr.Type = ICMP_ECHOREQ;
    echoReq.icmpHdr.Code = 0;
    echoReq.icmpHdr.Checksum = 0;
    echoReq.icmpHdr.ID = nId++;
    echoReq.icmpHdr.Seq = nSeq++;
    for (nRet = 0; nRet < REQ_DATASIZE; nRet++) {
        echoReq.cData[nRet] = '1' + nRet;
    }
    echoReq.dwTime = GetTickCount();
    echoReq.icmpHdr.Checksum = checksum((u_short * ) & echoReq, sizeof(ECHOREQUEST));
    nRet = sendto(s, (LPSTR) & echoReq, sizeof(ECHOREQUEST), 0, (struct sockaddr *) lpstToAddr, sizeof(SOCKADDR_IN));
    if (nRet == SOCKET_ERROR) {
        printf("send to() error:%d\n", WSAGetLastError());
    }
    return (nRet);
}

//接收应答回复并进行解析
DWORD RecvEchoReply(SOCKET s, LPSOCKADDR_IN lpsaFrom, u_char *pTTL) {
    ECHOREPLY echoReply;
    int nRet;
    int nAddrLen = sizeof(struct sockaddr_in);
    nRet = recvfrom(s, (LPSTR) & echoReply, sizeof(ECHOREPLY), 0, (LPSOCKADDR) lpsaFrom, &nAddrLen);
    if (nRet == SOCKET_ERROR) {
        printf("recvfrom() error:%d\n", WSAGetLastError());
    }
    *pTTL = echoReply.ipHdr.TTL;
    return (echoReply.echoRequest.dwTime);
}

//等待回应答复 ,使用 select 模型
int WaitForEchoReply(SOCKET s) {
    struct timeval timeout;
    fd_set readfds;
    readfds.fd_count = 1;
    readfds.fd_array[0] = s;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    return (select(1, &readfds, NULL, NULL, &timeout));
}

char * do_ping(){
    write_log_file("start do_ping");

    for (int  i= 0; i <10 ; ++i) {
        pingIpsResults[i]=NULL;
    }
    ipPingCount=0;

    int length = strlen(vpe_ips);
    int num=0;

    for (int i = 0; i < length; i++) {
        if (strlen(vpe_ips[i]) != 0){
            //write_log_file(vpe_ips[i]);
            num++;
        }
    }
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(1, 1), &wsd) != 0) {
        write_log_file("加载 Winsock 失败 !");
        return;
    }
    HANDLE hth[num];
    unsigned ThreadID[num];
    DWORD ExitCode[num];
    for (int k = 0; k < num; ++k) {
        if (strlen(vpe_ips[k]) != 0){
            hth[k] = (HANDLE) _beginthreadex(NULL, 0, Ping, vpe_ips[k], CREATE_SUSPENDED, &ThreadID[k]);
            GetExitCodeThread(hth[k], &ExitCode[k]);
            ResumeThread(hth[k]);
            WaitForSingleObject(hth[k], INFINITE);
            GetExitCodeThread(hth[k], &ExitCode[k]);
            CloseHandle(hth[k]);
        }
    }
    WSACleanup();

    cJSON *jsonStr,*js_body;
    jsonStr=cJSON_CreateObject();
    cJSON_AddItemToObject(jsonStr,"ipPacketStructs",js_body = cJSON_CreateArray());
    //cJSON_AddItemToArray(js_body, js_list = cJSON_CreateObject());
    for (int j = 0; j < 10; ++j) {
        cJSON_AddItemToArray(js_body, pingIpsResults[j]);
    }
    //char *pingResult = cJSON_PrintUnformatted(root);

    char *s = (char *) malloc(5000 * sizeof(char));
    strcpy(s, cJSON_PrintUnformatted(jsonStr));

    if(jsonStr){
        free(jsonStr);
    }
    write_log_file("end do_ping");
    return s;
}

//PING 功能实现
DWORD WINAPI Ping(LPVOID lpParam)
{
    char * strHost =(char *) lpParam;
    SOCKET rawSocket;
    LPHOSTENT lpHost;
    struct sockaddr_in destIP;
    struct sockaddr_in srcIP;
    DWORD dwTimeSent;
    DWORD dwElapsed;
    DWORD totalTime;
    u_char cTTL;
    int nLoop, k = 4;
    int nRet;
    int sent = 4, reveived = 0, lost = 0;
    double minimum = 100000.0, maximum = 0.0, average = 0.0, dev1 = 0.0, dev2 = 0.0, deviation = 0.0;
    cJSON *js_body, *js_list;
    rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (rawSocket == SOCKET_ERROR) {
        printf("socket() error:%d\n", WSAGetLastError());
        return;
    }
    lpHost = gethostbyname(strHost);
    if (lpHost == NULL) {
        printf("Host not found:%s\n", strHost);
        return;
    }
    destIP.sin_addr.s_addr = *((u_long FAR *)(lpHost->h_addr));
    destIP.sin_family = AF_INET;
    destIP.sin_port = 0;
    for (nLoop = 0; nLoop < k; nLoop++) {
        SendEchoRequest(rawSocket, &destIP);
        nRet = WaitForEchoReply(rawSocket);
        if (nRet == SOCKET_ERROR) {
            printf("select() error:%d\n", WSAGetLastError());
            break;
        }
        if (!nRet) {
            lost++;
            printf("\nRequest time out.");
            continue;
        }
        dwTimeSent = RecvEchoReply(rawSocket, &srcIP, &cTTL);
        reveived++;
        dwElapsed = GetTickCount() - dwTimeSent;
        if (dwElapsed > maximum) maximum = dwElapsed;
        if (dwElapsed < minimum) minimum = dwElapsed;
        if(maximum <= 0){
            maximum = 1;
        }
        if(minimum <= 0){
            minimum = 1;
        }
        totalTime += dwElapsed;
        average = totalTime / sent;
        if(average <= 0){
            average = 1;
        }
        dev1 = (average - minimum) / average;
        dev2 = (maximum - average) / average;
        if (dev1 >= dev2){
            deviation = dev1;
        } else {
            deviation = dev2;
        }
    }

    js_body = cJSON_CreateObject();
    cJSON_AddStringToObject(js_body,"target",strHost);
    cJSON_AddNumberToObject(js_body,"transmitted",sent);
    cJSON_AddNumberToObject(js_body,"received",reveived);
    cJSON_AddNumberToObject(js_body,"time",totalTime);
    cJSON_AddNumberToObject(js_body,"loss",lost);
    cJSON_AddNumberToObject(js_body,"min",minimum);
    cJSON_AddNumberToObject(js_body,"avg",average);
    cJSON_AddNumberToObject(js_body,"max",maximum);
    cJSON_AddNumberToObject(js_body,"deviation",deviation);

    pingIpsResults[ipPingCount] = js_body;

    ipPingCount++;

    nRet = closesocket(rawSocket);
    if (nRet == SOCKET_ERROR) {
        printf("closesocket() error:%d\n", WSAGetLastError());
    }
}