#include "cxp_main.h"

int cxp_start(){
    for( int i=0; i<10; i++ )
    {
        /*if (strcmp(secret_key,"")==0){
            write_log_file("save key");
            save_key();
            write_log_file("save key end");
        }*/

        // 点击连接按钮，获取到key后触发
        if(!cpe_deleted){
            if(!cpe_started){
                write_log_file("cpe start");
                ce_startup();
                write_log_file("cpe start end");
            }

            if(cpe_started && (!cpe_connected)){
                write_log_file("connect to vpe");
                connect_to_vpe();
                write_log_file("connect to vpe end");
            }

            if(cpe_started && cpe_connected){
                return 1;
            }

        }else if(cpe_deleted){
            return 0;
        }

    }
    return 0;
}

DWORD WINAPI cxp_main(LPVOID lpParam){
//void cxp_main(){
    for( ; ; )
    {
        // 连接成功后起线程发起心跳
        //write_log_file("ce_echo start");

        if(ce_echo()){
            Sleep(20000);
        }else{
            Sleep(10000);
        }

        //write_log_file("ce_echo end");
    }
}

void countThread(void* pArg)
{
    _beginthread(cxp_main,0,0);
    Sleep(1000);
}

void cxp_echo(){
    _beginthread(countThread,0,0);
//    Sleep(1000);
    /*HANDLE hth1;
    unsigned Thread1ID;

    hth1 = (HANDLE)_beginthreadex(NULL, 0, cxp_main,NULL, CREATE_SUSPENDED, &Thread1ID);

    DWORD ExitCode1;
    GetExitCodeThread(hth1, &ExitCode1);
    ResumeThread(hth1);
    WaitForSingleObject(hth1, INFINITE);
    GetExitCodeThread(hth1, &ExitCode1);
    CloseHandle(hth1);*/
}
