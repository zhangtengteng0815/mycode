/*
 *  OpenVPN-GUI -- A Windows GUI for OpenVPN.
 *
 *  Copyright (C) 2004 Mathias Sundman <mathias@nilings.se>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program (see the file COPYING included with this
 *  distribution); if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <process.h>
//#include <winbase.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if !defined (UNICODE)
#error UNICODE and _UNICODE must be defined. This version only supports unicode builds.
#endif

#include <windows.h>
#include <shlwapi.h>
#include <wtsapi32.h>
#include <prsht.h>
#include <pbt.h>
#include <commdlg.h>

#include "tray.h"
#include "openvpn.h"
#include "openvpn_config.h"
#include "viewlog.h"
#include "service.h"
#include "main.h"
#include "options.h"
#include "passphrase.h"
#include "proxy.h"
#include "registry.h"
#include "openvpn-gui-res.h"
#include "localization.h"
#include "manage.h"
#include "misc.h"
#include "save_pass.h"

#include "cobj.h"
#include "MD5.h"
#include "cJSON/cJSON.h"
#include "cxp_main.h"
#include "syscxp.h"
#include "logger.h"
#include "metadata.h"


#ifndef DISABLE_CHANGE_PASSWORD
#include <openssl/evp.h>
#include <openssl/err.h>
#endif

#define OVPN_EXITCODE_ERROR      1
#define OVPN_EXITCODE_TIMEOUT    2
#define OVPN_EXITCODE_NOTREADY   3

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
static void ShowSettingsDialog();
void CloseApplication(HWND hwnd);
void ImportConfigFile();

/*  Class name and window title  */
TCHAR szClassName[ ] = _T("犀思云-GUI");
TCHAR szTitleText[ ] = _T("VPN");

/* Options structure */
options_t o;


/* Workaround for ASLR on Windows */
__declspec(dllexport) char aslr_workaround;

static int
VerifyAutoConnections()
{
    int i;

    for (i = 0; i < o.num_auto_connect; i++)
    {
        if (GetConnByName(o.auto_connect[i]) == NULL)
        {
            /* autostart config not found */
            ShowLocalizedMsg(IDS_ERR_AUTOSTART_CONF, o.auto_connect[i]);
            return FALSE;
        }
    }

    return TRUE;
}

/*
 * Send a copydata message corresponding to any --command action option specified
 * to the running instance and return success or error.
 */
static int
NotifyRunningInstance()
{
    /* Check if a previous instance has a window initialized
     * Even if we are not the first instance this may return null
     * if the previous instance has not fully started up
     */
    HANDLE hwnd_master = FindWindow (szClassName, NULL);
    int exit_code = 0;
    if (hwnd_master)
    {
        /* GUI up and running -- send a message if any action is pecified,
           else show the balloon */
        COPYDATASTRUCT config_data = {0};
        int timeout = 30*1000;             /* 30 seconds */
        if (!o.action)
        {
            o.action = WM_OVPN_NOTIFY;
            o.action_arg = LoadLocalizedString(IDS_NFO_CLICK_HERE_TO_START);
        }
        config_data.dwData = o.action;
        if (o.action_arg)
        {
            config_data.cbData = (wcslen(o.action_arg)+1)*sizeof(o.action_arg[0]);
            config_data.lpData = (void *) o.action_arg;
        }
        PrintDebug(L"Instance 2: called with action %d : %s", o.action, o.action_arg);
        if (!SendMessageTimeout (hwnd_master, WM_COPYDATA, 0,
                                 (LPARAM) &config_data, 0, timeout, NULL))
        {
            DWORD error = GetLastError();
            if (error == ERROR_TIMEOUT)
            {
                exit_code = OVPN_EXITCODE_TIMEOUT;
                MsgToEventLog(EVENTLOG_ERROR_TYPE, L"Sending command to running instance timed out.");
            }
            else
            {
                exit_code = OVPN_EXITCODE_ERROR;
                MsgToEventLog(EVENTLOG_ERROR_TYPE, L"Sending command to running instance failed (error = %lu).",
                              error);
            }
        }
    }
    else
    {
        /* An instance is already running but its main window not yet initialized */
        exit_code = OVPN_EXITCODE_NOTREADY;
        MsgToEventLog(EVENTLOG_ERROR_TYPE, L"Previous instance not yet ready to accept commands. "
                      "Try again later.");
    }

    return exit_code;
}

int WINAPI _tWinMain (HINSTANCE hThisInstance,
                    UNUSED HINSTANCE hPrevInstance,
                    UNUSED LPTSTR lpszArgument,
                    UNUSED int nCmdShow)
{
  MSG messages;            /* Here messages to the application are saved */
  WNDCLASSEX wincl;        /* Data structure for the windowclass */
  DWORD shell32_version;
  BOOL first_instance = TRUE;
  /* a session local semaphore to detect second instance */
  HANDLE session_semaphore = InitSemaphore(L"Local\\"PACKAGE_NAME);

  /* try to lock the semaphore, else we are not the first instance */
  if (session_semaphore &&
      WaitForSingleObject(session_semaphore, 200) != WAIT_OBJECT_0)
  {
      first_instance = FALSE;
  }

  /* Initialize handlers for manangement interface notifications */
  mgmt_rtmsg_handler handler[] = {
      { ready,    OnReady },
      { hold,     OnHold },
      { log,      OnLogLine },
      { state,    OnStateChange },
      { password, OnPassword },
      { proxy,    OnProxy },
      { stop,     OnStop },
      { needok,   OnNeedOk },
      { needstr,  OnNeedStr },
      { echo,     OnEcho },
      { bytecount,OnByteCount },
      { 0,        NULL }
  };
  InitManagement(handler);

  /* initialize options to default state */
  InitOptions(&o);
  if (first_instance)
      o.session_semaphore = session_semaphore;

#ifdef DEBUG
  /* Open debug file for output */
  if (!(o.debug_fp = _wfopen(DEBUG_FILE, L"a+,ccs=UTF-8")))
    {
      /* can't open debug file */
      ShowLocalizedMsg(IDS_ERR_OPEN_DEBUG_FILE, DEBUG_FILE);
      exit(1);
    }
  PrintDebug(_T("Starting OpenVPN GUI v%S"), PACKAGE_VERSION);
#endif


  o.hInstance = hThisInstance;

  if(!GetModuleHandle(_T("RICHED20.DLL")))
    {
      LoadLibrary(_T("RICHED20.DLL"));
    }
  else
    {
      /* can't load riched20.dll */
      ShowLocalizedMsg(IDS_ERR_LOAD_RICHED20);
      exit(1);
    }

  /* Check version of shell32.dll */
  shell32_version=GetDllVersion(_T("shell32.dll"));
  if (shell32_version < PACKVERSION(5,0))
    {
      /* shell32.dll version to low */
      ShowLocalizedMsg(IDS_ERR_SHELL_DLL_VERSION, shell32_version);
      exit(1);
    }
#ifdef DEBUG
  PrintDebug(_T("Shell32.dll version: 0x%lx"), shell32_version);
#endif

  if (first_instance)
      UpdateRegistry(); /* Checks version change and update keys/values */

  GetRegistryKeys();
  /* Parse command-line options */
  ProcessCommandLine(&o, GetCommandLine());


    char cmdline[50];
    //strcpy(cmdline, "rd /s /q ");
    strcpy(cmdline, "/k rd /s /q ");

    char configDir[30];
    sprintf(configDir, "%ws", o.config_dir );

    strcat( cmdline, configDir);

    //system(cmdline);
    ShellExecuteW(NULL, NULL, L"cmd.exe", cmdline , NULL, SW_HIDE);


    TCHAR filename[2 * MAX_PATH];
    //TCHAR configName[] = TEXT("");
    TCHAR configFile[] = TEXT("client.ovpn");
    FILE* fp = NULL;
    _sntprintf_0(filename, _T("%s\\%s"), o.config_dir, configFile);

    fp = _tfopen(filename, TEXT("a+"));
    fclose(fp);


    /*删除config目录下所有文件*/
    //TCHAR cmdline[MAX_PATH];
    //_sntprintf_0(cmdline, _T("%s \"%s\""), _T("rd /s /q "), o.config_dir);
    //_sntprintf_0(filename, _T("%s \"%s\\%s\""), o.editor, o.conn[config].config_dir, o.conn[config].config_file);
    //_sntprintf_0(cmdline, _T("rd /s /q %s"), o.config_dir);

    //char str[30];
    //sprintf(str,"%s",o.config_dir);
    //sprintf(str, "%ws", cmdline );
    //strcpy(str,o.config_dir);
    //write_log_file(str);

    //PROCESS_INFORMATION pi;

//    STARTUPINFO si;
//    si.cb = sizeof(STARTUPINFO);
//    si.lpReserved = NULL;
//    si.lpDesktop = NULL;
//    si.lpTitle = NULL;
//    //隐藏掉可能出现的cmd命令窗口
//    si.dwFlags = STARTF_USESHOWWINDOW;
//    si.wShowWindow = SW_HIDE;
//    si.cbReserved2 = NULL;
//    si.lpReserved2 = NULL;
//
//    CreateProcess(
//            "C:\\Windows\\system32\\cmd.exe",   //  指向一个NULL结尾的、用来指定可执行模块的宽字节字符串
//            cmdline,
//            NULL, //    指向一个SECURITY_ATTRIBUTES结构体，这个结构体决定是否返回的句柄可以被子进程继承。
//            NULL, //    如果lpProcessAttributes参数为空（NULL），那么句柄不能被继承。<同上>
//            false,//    指示新进程是否从调用进程处继承了句柄。
//            0,  //  指定附加的、用来控制优先类和进程的创建的标
//            NULL, //    指向一个新进程的环境块。如果此参数为空，新进程使用调用进程的环境
//            NULL, //    指定子进程的工作路径
//            &si, // 决定新进程的主窗体如何显示的STARTUPINFO结构体
//            &pi  // 接收新进程的识别信息的PROCESS_INFORMATION结构体
//    );

    //WaitForSingleObject( pi.hProcess, INFINITE );
    //CloseHandle( pi.hThread );
    //CloseHandle( pi.hProcess );

    EnsureDirExists(o.config_dir);

  if (!first_instance)
  {
      int res = NotifyRunningInstance();
      exit(res);
  }
  else if (o.action == WM_OVPN_START)
  {
      PrintDebug(L"Instance 1: Called with --command connect xxx. Treating it as --connect xxx");
  }
  else if (o.action)
  {
      MsgToEventLog(EVENTLOG_ERROR_TYPE, L"Called with --command when no previous instance available");
      exit(OVPN_EXITCODE_ERROR);
  }

  if (!CheckVersion()) {
    exit(1);
  }

  if (!EnsureDirExists(o.log_dir))
  {
    ShowLocalizedMsg(IDS_ERR_CREATE_PATH, _T("log_dir"), o.log_dir);
    exit(1);
  }


  sprintf(LOG_FILE, "%ws", o.log_dir );
  strcat(LOG_FILE, "\\log.txt");
  write_log_file("logfile======>>>>>");
  write_log_file(LOG_FILE);


  if (!IsUserAdmin() && strtod(o.ovpn_version, NULL) > 2.3 && !o.silent_connection)
    CheckIServiceStatus(TRUE);

  BuildFileList();
  if (!VerifyAutoConnections()) {
    exit(1);
  }

  GetProxyRegistrySettings();

#ifndef DISABLE_CHANGE_PASSWORD
  /* Initialize OpenSSL */
  OpenSSL_add_all_algorithms();
  ERR_load_crypto_strings();
#endif

  /* The Window structure */
  wincl.hInstance = hThisInstance;
  wincl.lpszClassName = szClassName;        /* 窗口类别名称 */
  wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows 窗口函数*/
  wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
  wincl.cbSize = sizeof (WNDCLASSEX);

  /* Use default icon and mouse-pointer */
  wincl.hIcon = LoadLocalizedIcon(ID_ICO_APP); /*窗口图标*/
  wincl.hIconSm = LoadLocalizedIcon(ID_ICO_APP);
  wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
  wincl.lpszMenuName = NULL;                 /* No menu 菜单*/
  wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
  wincl.cbWndExtra = 0;                      /* structure or the window instance */
  /* Use Windows's default color as the background of the window */
  wincl.hbrBackground = (HBRUSH) COLOR_3DSHADOW; //COLOR_BACKGROUND;

  /* Register the window class, and if it fails quit the program */
  if (!RegisterClassEx (&wincl))
    return 1;

  /* The class is registered, let's create the program*/
  CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname 类名，要和刚才注册的一致*/
           szTitleText,         /* Title Text 窗口标题 */
           WS_OVERLAPPEDWINDOW, /* default window 窗口外观样式*/
           (int)CW_USEDEFAULT,  /* Windows decides the position 窗口相对于父级的X坐标*/
           (int)CW_USEDEFAULT,  /* where the window ends up on the screen 窗口相对于父级的Y坐标*/
           230,                 /* The programs width 窗口的宽度*/
           200,                 /* and height in pixels 窗口的高度*/
           HWND_DESKTOP,        /* The window is a child-window to desktop 没有父窗口，为NULL*/
           NULL,                /* No menu 没有菜单，为NULL*/
           hThisInstance,       /* Program Instance handler当前应用程序的实例句柄 */
           NULL                 /* No Window Creation data 没有附加数据，为NUL*/
           );


  /* Run the message loop. It will run until GetMessage() returns 0 */
  while (GetMessage (&messages, NULL, 0, 0))
  {
    TranslateMessage(&messages);
    DispatchMessage(&messages);
  }

  CloseSemaphore(o.session_semaphore);
  o.session_semaphore = NULL;   /* though we're going to die.. */
  if (o.event_log)
      DeregisterEventSource(o.event_log);

  /* The program return-value is 0 - The value that PostQuitMessage() gave */
  return messages.wParam;
}


static void
StopAllOpenVPN()
{
    int i;

    for (i = 0; i < o.num_configs; i++)
    {
        if (o.conn[i].state != disconnected)
            StopOpenVPN(&o.conn[i]);
    }

    /* Wait for all connections to terminate (Max 5 sec) */
    for (i = 0; i < 20; i++, Sleep(250))
    {
        if (CountConnState(disconnected) == o.num_configs)
            break;
    }
}


static int
AutoStartConnections()
{
    int i;

    for (i = 0; i < o.num_configs; i++)
    {
        if (o.conn[i].auto_connect)
            StartOpenVPN(&o.conn[i]);
    }

    return TRUE;
}


static void
ResumeConnections()
{
    int i;
    for (i = 0; i < o.num_configs; i++) {
        /* Restart suspend connections */
        if (o.conn[i].state == suspended)
            StartOpenVPN(&o.conn[i]);

        /* If some connection never reached SUSPENDED state */
        if (o.conn[i].state == suspending)
            StopOpenVPN(&o.conn[i]);
    }
}

/*
 * Set scale factor of windows in pixels. Scale = 100% for dpi = 96
 */
static void
dpi_setscale(UINT dpix)
{
    /* scale factor in percentage compared to the reference dpi of 96 */
    if (dpix != 0)
        o.dpi_scale = MulDiv(dpix, 100, 96);
    else
        o.dpi_scale = 100;
    PrintDebug(L"DPI scale set to %u", o.dpi_scale);
}

/*
 * Get dpi of the system and set the scale factor.
 * The system dpi may be different from the per monitor dpi on
 * Win 8.1 later. We set dpi awareness to system-dpi level in the
 * manifest, and let Windows automatically re-scale windows
 * if/when dpi changes dynamically.
 */
static void
dpi_initialize(void)
{
    UINT dpix = 0;
    HDC hdc = GetDC(NULL);

    if (hdc)
    {
        dpix = GetDeviceCaps(hdc, LOGPIXELSX);
        ReleaseDC(NULL, hdc);
        PrintDebug(L"System DPI: dpix = %u", dpix);
    }
    else
    {
        PrintDebug(L"GetDC failed, using default dpi = 96 (error = %lu)", GetLastError());
        dpix = 96;
    }

    dpi_setscale(dpix);
}

static int
HandleCopyDataMessage(const COPYDATASTRUCT *copy_data)
{
    WCHAR *str = NULL;
    connection_t *c = NULL;
    PrintDebug (L"WM_COPYDATA message received. (dwData: %lu, cbData: %lu, lpData: %s)",
                copy_data->dwData, copy_data->cbData, copy_data->lpData);
    if (copy_data->cbData >= sizeof(WCHAR) && copy_data->lpData)
    {
        str = (WCHAR*) copy_data->lpData;
        str[copy_data->cbData/sizeof(WCHAR)-1] = L'\0'; /* in case not nul-terminated */
        c = GetConnByName(str);
    }
    if(copy_data->dwData == WM_OVPN_START && c)
    {
        if (!o.silent_connection)
            ForceForegroundWindow(o.hWnd);
        StartOpenVPN(c);
    }
    else if(copy_data->dwData == WM_OVPN_STOP && c)
        StopOpenVPN(c);
    else if(copy_data->dwData == WM_OVPN_RESTART && c)
    {
        if (!o.silent_connection)
            ForceForegroundWindow(o.hWnd);
        RestartOpenVPN(c);
    }
    else if(copy_data->dwData == WM_OVPN_SHOWSTATUS && c && c->hwndStatus)
    {
        ForceForegroundWindow(o.hWnd);
        ShowWindow(c->hwndStatus, SW_SHOW);
    }
    else if(copy_data->dwData == WM_OVPN_STOPALL)
        StopAllOpenVPN();
    else if(copy_data->dwData == WM_OVPN_SILENT && str)
    {
        if (_wtoi(str) == 0)
            o.silent_connection = 0;
        else
            o.silent_connection = 1;
    }
    else if(copy_data->dwData == WM_OVPN_EXIT)
    {
        CloseApplication(o.hWnd);
    }
    /* Not yet implemented
    else if(copy_data->dwData == WM_OVPN_IMPORT)
    {
    }
    */
    else if (copy_data->dwData == WM_OVPN_NOTIFY)
    {
        ShowTrayBalloon(L"", copy_data->lpData);
    }
    else
    {
        MsgToEventLog(EVENTLOG_ERROR_TYPE,
                      L"Unknown WM_COPYDATA message ignored. (dwData: %lu, cbData: %lu, lpData: %s)",
                      copy_data->dwData, copy_data->cbData, copy_data->lpData);
        return FALSE;
    }
    return TRUE; /* indicate we handled the message */
}


// 处理登陆
LRESULT CALLBACK LoginProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch (msg)
    {
    case WM_SYSCOMMAND:
        if (wParam == SC_CLOSE)
        {
            // 如果执行了关闭
            // 销毁对话框，将收到WM_DESTROY消息
            DestroyWindow(hdlg);
            PostQuitMessage(0);
        }
        return 0;
    case WM_CLOSE:
        DestroyWindow(hdlg);
        PostQuitMessage(0);
        break;
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case ID_SAVE_PASS_LOGIN:
        {
            break;
        }
        case ID_HTTPS: {
            //system(" test1.exe \"http://192.168.211.71:8080/post\" {\\\"name\\\":\\\"sam\\\",\\\"age\\\":20} ");
            /*测试http请求*/
            /*ft_http_client_t* http = 0;
            ft_http_init();
            http = ft_http_new();
            const char* body = 0;
            body = ft_http_sync_post_request(http, "http://192.168.211.71:8080/post", "{\"name\": \"sam\",\"age\" : 20}",M_POST);
            */

            /*测试json2obj*/
            /*
            REGISTER_METAINFO(User);
            CSTR json = CS(body);
            struct User *user = OBJECT_NEW(User);
            user = OBJECT_FROM_JSON(User, json.cstr);

            MessageBoxA(hdlg,  body,body, MB_OK);
            */

            /*测试obj2json*/
            /*struct User *rec = OBJECT_NEW(User);
            rec->age = 2014;
            rec->name = CS("zhangtengteng"); // CSTR must be initialized with 'CS' macro
            json = OBJECT_TO_JSON(rec, User);
            MessageBoxA(hdlg,  json.cstr,json.cstr, MB_OK);
            // clear object but not free
            OBJECT_CLEAR(rec, User);
            // delete 'rec' (free allocated memory), instead of 'free' function
            OBJECT_DELETE(rec, User);
            */

            //ping ceshi
           /* WSADATA wsd;
            if (WSAStartup(MAKEWORD(1, 1), &wsd) != 0) {
                printf(" 加载 Winsock 失败 !\n");
            }

            char * pingIps[] = {"112.80.248.76",
                                "101.71.100.123",
                                "216.58.200.4",
                                "140.207.205.39"};

            HANDLE hth1, hth2, hth3, hth4;
            unsigned Thread1ID;
            unsigned Thread2ID;
            unsigned Thread3ID;
            unsigned Thread4ID;

            hth1 = (HANDLE)_beginthreadex(NULL, 0, Ping, pingIps[0], CREATE_SUSPENDED, &Thread1ID);
            hth2 = (HANDLE)_beginthreadex(NULL, 0, Ping, pingIps[1], CREATE_SUSPENDED, &Thread2ID);
            hth3 = (HANDLE)_beginthreadex(NULL, 0, Ping, pingIps[2], CREATE_SUSPENDED, &Thread3ID);
            hth4 = (HANDLE)_beginthreadex(NULL, 0, Ping, pingIps[3], CREATE_SUSPENDED, &Thread4ID);
            //    if (hth1== 0)
            //        printf("Failed to create thread 1\n");

            DWORD ExitCode1;
            GetExitCodeThread(hth1, &ExitCode1);
            ResumeThread(hth1);
            WaitForSingleObject(hth1, INFINITE);
            GetExitCodeThread(hth1, &ExitCode1);
            printf("thread 1 exited with code %u, its pid = %u, its handle = %d \n", ExitCode1, Thread1ID, hth1);
            CloseHandle(hth1);

            DWORD ExitCode2;
            GetExitCodeThread(hth2, &ExitCode2);
            ResumeThread(hth2);
            WaitForSingleObject(hth2, INFINITE);
            GetExitCodeThread(hth2, &ExitCode2);
            printf("thread 2 exited with code %u, its pid = %u, its handle = %d \n", ExitCode2, Thread2ID, hth2);
            CloseHandle(hth2);

            DWORD ExitCode3;
            GetExitCodeThread(hth3, &ExitCode3);
            ResumeThread(hth3);
            WaitForSingleObject(hth3, INFINITE);
            GetExitCodeThread(hth3, &ExitCode3);
            printf("thread 3 exited with code %u, its pid = %u, its handle = %d \n", ExitCode3, Thread3ID, hth3);
            CloseHandle(hth3);

            DWORD ExitCode4;
            GetExitCodeThread(hth4, &ExitCode4);
            ResumeThread(hth4);
            WaitForSingleObject(hth4, INFINITE);
            GetExitCodeThread(hth4, &ExitCode4);
            printf("thread 4 exited with code %u, its pid = %u, its handle = %d \n", ExitCode4, Thread4ID, hth4);
            CloseHandle(hth4);

            int j=0;
            int i;
            char str[100];
            for (i = 0; i < 4; i++){
                j+=sprintf(str+j, "%s, %d, %d, %d, %d, %f, %f, %f, %f\n", pingIpsResults[i].target, pingIpsResults[i].transmitted, pingIpsResults[i].received,
                        pingIpsResults[i].time, pingIpsResults[i].loss, pingIpsResults[i].min, pingIpsResults[i].avg, pingIpsResults[i].max, pingIpsResults[i].deviation);
            }

            MessageBoxA(hdlg, str, str, MB_OK);

            WSACleanup();
           */

            /*测试MD5*/
            /*char msg[]="zhangsan";
            uint8_t result[16];
            md5((uint8_t*)msg, strlen(msg), result);
            char md5str[80];
            int j=0;
            int i;
            for (i = 0; i < 16; i++){
                j+=sprintf(md5str+j, "%2.2x", result[i]);
            }
            MessageBoxA(hdlg,  md5str,md5str, MB_OK);
            */

            /*测试message*/
           /*REGISTER_METAINFO(APIGetMobileSecretKeyMsg);
            struct APIGetMobileSecretKeyMsg *msg1 = OBJECT_NEW(APIGetMobileSecretKeyMsg);
            msg1->esn = CS("pc");
            msg1->password = CS("jnznS8Vn6J");
            msg1->signature = CS("123");
            CSTR json = OBJECT_TO_JSON(msg1, APIGetMobileSecretKeyMsg);

            char msgname[] = "APIGetMobileSecretKeyMsg";
            char * msgres=assemble_msg(&msgname, json.cstr);*/


            //MessageBoxA(hdlg,  msgres,msgres, MB_OK);
            //char * body=call_server(msgres);
            //MessageBoxA(hdlg,  body,body, MB_OK);


           /* REGISTER_METAINFO(APICeCmdCallbackMsg);
            struct APICeCmdCallbackMsg *msg2 = OBJECT_NEW(APICeCmdCallbackMsg);
            msg2->esn = CS("pc");
            msg2->timestamp = CS("123434");
            msg2->jsonStr = CS("wewsfassafsa");
            msg2->signature = CS("123");
            json = OBJECT_TO_JSON(msg2, APICeCmdCallbackMsg);

            char msgname2[] = "APICeCmdCallbackMsg";
            char * msgres2=assemble_msg(&msgname2, json.cstr);*/
            //MessageBoxA(hdlg,  msgres2,msgres2, MB_OK);
            //write_log_file("start");
            //Sleep(3000);
            //free(msgres2);
            //write_log_file("end");
            //Sleep(3000);

            /*HANDLE hth1;
            unsigned Thread1ID;

            hth1 = (HANDLE)_beginthreadex(NULL, 0, call_server,msgres, CREATE_SUSPENDED, &Thread1ID);

            DWORD ExitCode1;
            GetExitCodeThread(hth1, &ExitCode1);
            ResumeThread(hth1);
            WaitForSingleObject(hth1, INFINITE);
            GetExitCodeThread(hth1, &ExitCode1);
            //printf("thread 1 exited with code %u, its pid = %u, its handle = %d \n", ExitCode1, Thread1ID, hth1);
            CloseHandle(hth1);*/


            //HANDLE handle = CreateThread(NULL, 0, call_server, msgres, 0, NULL);
            //WaitForSingleObject(handle, INFINITE);

            //call_server(msgres);
            //char * body=call_server(msgres);
            //free(msgres);
            /*while (msgres)
            {
                free(msgres);
                msgres = NULL;
            }*/
            //free(ser_url);
            //MessageBoxA(hdlg,  body,body, MB_OK);



            /*ft_http_client_t* http = 0;
            ft_http_init();
            http = ft_http_new();
            const char* body = 0;
            body = ft_http_sync_post_request(http, "http://192.168.211.73:8089/sdwan/api",msgres,M_POST);
            free(msgres);
            MessageBoxA(hdlg,  body,body, MB_OK);*/


            /*char * reply_name = "APIGetMobileSecretKeyReply";
            char *reply = unassemble_reply(reply_name, body);
            MessageBoxA(hdlg, cJSON_Print(reply), cJSON_Print(reply), MB_OK);*/

            //free(reply);
//            REGISTER_METAINFO(APIReply);
//            CSTR reply_json = CS(body);
//            struct APIReply *api_reply = OBJECT_NEW(APIReply);
//            api_reply = OBJECT_FROM_JSON(APIReply, reply_json.cstr);


//            struct APIGetMobileSecretKeyReply *reply1 = OBJECT_NEW(APIGetMobileSecretKeyReply);
//            reply1 = OBJECT_FROM_JSON(APIGetMobileSecretKeyReply, json.cstr);
//            MessageBoxA(hdlg, reply1->secretKey, reply1->esn, MB_OK);

            //MessageBoxA(hdlg, LOG_FILE, LOG_FILE, MB_OK);
            //write_log_file("cpe start");
            cxp_start();
            cxp_echo();


            return TRUE;
        }
        case ID_LOGIN: {
            //创建openvpn的配置文件
            FILE* fp = NULL;
            TCHAR filename[2 * MAX_PATH];

            _sntprintf_0(filename, _T("%s\\%s\\%s"), o.config_dir, o.conn[config].config_name, o.conn[config].config_file);

            //MessageBox(hdlg, filename, NULL, MB_OK);

          /*
            if ((fp = _tfopen(filename, TEXT("w+"))) != NULL)
            {
                fputs("client\n", fp);
                fputs("dev tun\n", fp);
                fputs("proto udp\n", fp);
                fputs("remote 192.168.211.71 1194\n", fp);
                fputs("ping 5\n", fp);
                fputs("comp-lzo\n", fp);
                fputs("nobind\n", fp);
                fputs("cipher AES-256-CBC\n", fp);
                fputs("verb 4\n", fp);
                fputs("mute 20\n", fp);
                fputs("auth-user-pass\n", fp);
                fputs(";log\n", fp);
                fputs(";log-append\n", fp);
                fputs("proto udp\n", fp);
                fputs("<ca>\n", fp);
                fputs("CA\n", fp);
                fputs("</ca>\n", fp);
                fputs("<cert>\n", fp);
                fputs("Certificate\n", fp);
                fputs("</cert>\n", fp);
                fputs("<key>\n", fp);
                fputs("KEY\n", fp);
                fputs("</key>\n", fp);

            }
            */
            fclose(fp);



            //MessageBox(hdlg, o.conn[config].log_path, o.log_dir, MB_OK);

            //log_path==C:\Users\user\OpenVPN\log\client.log
            //log_dir==C:\Users\user\OpenVPN\log\

            //config_file==client.ovpn;
            //config_name==client;
            //config_dir==C:\Users\user\OpenVPN\config;

            //MessageBox(hdlg, filename, o.editor, MB_OK);
            //MessageBox(hdlg, o.config_dir, o.conn[config].config_file, MB_OK);
            //MessageBox(hdlg, o.conn[config].config_name, o.config_dir, MB_OK);


            HWND child = GetDlgItem(hdlg, ID_SAVE_PASS_LOGIN);
            int ret = SendMessage(child, BM_GETCHECK, 0, 0);

            if (ret != BST_CHECKED)
            {
                _sntprintf_0(filename, _T("%s\\%s\\%s"), o.config_dir, o.conn[config].config_name, TEXT("password"));

                if (remove(filename) == 0)
                 {
                    //MessageBox(hdlg, TEXT("ok"), NULL, MB_OK);
                }
                else {
                    //MessageBox(hdlg, TEXT("faild"), NULL, MB_OK);
                    fp = _tfopen(filename, TEXT("w+"));
                    feof(fp);
                    fclose(fp);
                }
                /*
                if ((fp = _tfopen(filename, TEXT("w+"))) != NULL)
                {

                    strcpy(user.id, TEXT(""));
                    strcpy(user.pwd, TEXT(""));

                    fwrite(&user, sizeof(struct The_users), 1, fp);

                    //fwrite(login_username, sizeof(TCHAR), wcslen(login_username), fp);
                    //fwrite("\n", sizeof(TCHAR), wcslen("\n"), fp);
                    //fwrite(login_password, sizeof(TCHAR), wcslen(login_password), fp);
                    //fputs(login_username, fp);
                    //fputs(L"\n", fp);
                    //fputs(login_password, fp);
                }
                fclose(fp);
                */
                //SendMessage(child, BM_SETCHECK, BST_UNCHECKED, 0);

            }
            else
            {
                HWND hw;
                syscxpuser user;//结构体 The_users 重命名定义
                hw = GetDlgItem(hdlg, ID_USER_LOGIN);
                GetWindowText(hw, user.id, 100);
                hw = GetDlgItem(hdlg, ID_PASS_LOGIN);
                GetWindowText(hw, user.pwd, 100);
                //MessageBox(hdlg, user.id, NULL, MB_OK);
                //MessageBox(hdlg, user.pwd, NULL, MB_OK);
                _sntprintf_0(filename, _T("%s\\%s\\%s"), o.config_dir, o.conn[config].config_name, TEXT("password"));
                if ((fp = _tfopen(filename, TEXT("w+"))) != NULL)
                {
                    fwrite(&user, sizeof(struct The_user), 1, fp);

                    //fwrite(login_username, sizeof(TCHAR), wcslen(login_username), fp);
                    //fwrite("\n", sizeof(TCHAR), wcslen("\n"), fp);
                    //fwrite(login_password, sizeof(TCHAR), wcslen(login_password), fp);
                    //fputs(login_username, fp);
                    //fputs(L"\n", fp);
                    //fputs(login_password, fp);
                }
                fclose(fp);
                //SendMessage(child, BM_SETCHECK, BST_CHECKED, 0);
            }

            DestroyWindow(hdlg);
            break;
        }
        case ID_CANCEL:
            DestroyWindow(hdlg);
            PostQuitMessage(0);
            break;
        }

    }
    return 0;
    }
    return (INT_PTR)FALSE;
}


/*  This function is called by the Windows function DispatchMessage()  */
LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  static UINT s_uTaskbarRestart;
  //HWND hTime;

  switch (message) {

    /*case WM_TIMER:
    {
        switch (wParam)
        {
        case ID_TIMER:
        {
            char sztime;
            SYSTEMTIME time;
            GetLocalTime(&time);
            sprintf(sztime, "%d:%d:%d", time.wHour, time.wMinute, time.wSecond);

            MessageBox(hwnd, sztime, NULL, MB_OK);
            hTime = GetDlgItem(hwnd, ID_TXT_TIME);
            SetWindowText(hTime, sztime);
            break;
        }
        default:
            break;
        }
        break;
    }*/
    case WM_CREATE:
    {
        //char sztime[20];
        //SYSTEMTIME time;
        //GetLocalTime(&time);
        //sprintf(sztime, "%d:%d:%d", time.wHour, time.wMinute, time.wSecond);
        //SetTimer(hwnd, ID_TIMER, 1000, NULL);//设定时器

        //在窗体上创建一个Label标签
        //hTime = CreateWindowEx(0, RICHEDIT_CLASS, TEXT(sztime), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 48, 57, 16, hwnd, HMENU(ID_TXT_TIME), o.hInstance, NULL);


        /*HWND hd;
        HWND hw;
        FILE* fp = NULL;
        TCHAR filename[2 * MAX_PATH];
        syscxpuser user;//结构体 The_users 重命名定义
        //显示登陆
        hd = CreateDialog(o.hInstance, MAKEINTRESOURCE(IDD_LOGIN), hwnd, (DLGPROC)LoginProc);
        _sntprintf_0(filename, _T("%s\\%s\\%s"), o.config_dir, o.conn[config].config_name, TEXT("password"));
        //char buff[100];
        //MessageBox(hdlg, filename, NULL, MB_OK);
        if ((fp = _tfopen(filename, TEXT("r"))) != NULL)
        {
            fread(&user, sizeof(struct The_user), 1, fp);
            //fread(buff, sizeof(char), wcslen(buff), fp);
            //fgets(buff, 100, fp);
            hw = GetDlgItem(hd, ID_USER_LOGIN);
            //MessageBox(hd, user.id, NULL, MB_OK);
            SetWindowText(hw, user.id);
            //fread(buff, sizeof(char), wcslen(buff), fp);
            //fgets(buff, 100, fp);
            hw = GetDlgItem(hd, ID_PASS_LOGIN);
            //MessageBox(hd, user.pwd, NULL, MB_OK);
            SetWindowText(hw, user.pwd);
        }
        fclose(fp);
        //显示对话框
        ShowWindow(hd, SW_NORMAL);*/


        //cxp_start();
        //cxp_echo();

        // TODO
        OnPassword(o.conn, "Need 'Auth' username/password");

        //StartOpenVPN(&o.conn[0]);

       //Sleep(200000);

        //strcpy(o.conn[0].manage.password, secret_key);
        //o.conn[0].manage.password=secret_key;

      /* Save Window Handle */
      o.hWnd = hwnd;
      dpi_initialize();

      s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));

      WTSRegisterSessionNotification(hwnd, NOTIFY_FOR_THIS_SESSION);

      /* Load application icon */
      HICON hIcon = LoadLocalizedIcon(ID_ICO_APP);
      if (hIcon) {
        SendMessage(hwnd, WM_SETICON, (WPARAM) (ICON_SMALL), (LPARAM) (hIcon));
        SendMessage(hwnd, WM_SETICON, (WPARAM) (ICON_BIG), (LPARAM) (hIcon));
      }

      /* Enable next line to accept WM_COPYDATA messages from lower level processes */
#if 0
      ChangeWindowMessageFilterEx(hwnd, WM_COPYDATA, MSGFLT_ALLOW, NULL);
#endif

      CreatePopupMenus();	/* Create popup menus */
      ShowTrayIcon();
      if (o.service_only)
        CheckServiceStatus();	// Check if service is running or not

     /*if (!AutoStartConnections()) {
        SendMessage(hwnd, WM_CLOSE, 0, 0);
        break;
      }*/


 /*   OnNotifyTray(lParam); 	// Manages message from tray
    write_log_file("post message=========");
    SendMessage(hwnd, IDM_CONNECTMENU, 0, 0);*/


    self_start();


     /*
        char esnstr[50];
        sprintf(esnstr, "%ws", esn );

        char secret_key_str[35];
        strcpy(secret_key_str,secret_key);

        ManagementCommandFromSecretKey(&o.conn[0],"username \"Auth\" \"%s\"",esnstr);
        ManagementCommandFromSecretKey(&o.conn[0],"password \"Auth\" \"%s\"",secret_key_str);
      */

      break;
    }
    case WM_NOTIFYICONTRAY:

      OnNotifyTray(lParam); 	// Manages message from tray
      break;

    case WM_COPYDATA:   // custom messages with data from other processes
      HandleCopyDataMessage((COPYDATASTRUCT*) lParam);
      return TRUE; /* lets the sender free copy_data */

    case WM_COMMAND:
      if ( (LOWORD(wParam) >= IDM_CONNECTMENU) && (LOWORD(wParam) < IDM_CONNECTMENU + MAX_CONFIGS) ) {
        StartOpenVPN(&o.conn[LOWORD(wParam) - IDM_CONNECTMENU]);
      }
      if ( (LOWORD(wParam) >= IDM_DISCONNECTMENU) && (LOWORD(wParam) < IDM_DISCONNECTMENU + MAX_CONFIGS) ) {
        StopOpenVPN(&o.conn[LOWORD(wParam) - IDM_DISCONNECTMENU]);
      }
      if ( (LOWORD(wParam) >= IDM_RECONNECTMENU) && (LOWORD(wParam) < IDM_RECONNECTMENU + MAX_CONFIGS) ) {
        RestartOpenVPN(&o.conn[LOWORD(wParam) - IDM_RECONNECTMENU]);
      }
      if ( (LOWORD(wParam) >= IDM_STATUSMENU) && (LOWORD(wParam) < IDM_STATUSMENU + MAX_CONFIGS) ) {
        ShowWindow(o.conn[LOWORD(wParam) - IDM_STATUSMENU].hwndStatus, SW_SHOW);
      }
      if ( (LOWORD(wParam) >= IDM_VIEWLOGMENU) && (LOWORD(wParam) < IDM_VIEWLOGMENU + MAX_CONFIGS) ) {
        ViewLog(LOWORD(wParam) - IDM_VIEWLOGMENU);
      }
      if ( (LOWORD(wParam) >= IDM_EDITMENU) && (LOWORD(wParam) < IDM_EDITMENU + MAX_CONFIGS) ) {
        EditConfig(LOWORD(wParam) - IDM_EDITMENU);
      }
      if ( (LOWORD(wParam) >= IDM_CLEARPASSMENU) && (LOWORD(wParam) < IDM_CLEARPASSMENU + MAX_CONFIGS) ) {
        ResetSavePasswords(&o.conn[LOWORD(wParam) - IDM_CLEARPASSMENU]);
      }
#ifndef DISABLE_CHANGE_PASSWORD
      if ( (LOWORD(wParam) >= IDM_PASSPHRASEMENU) && (LOWORD(wParam) < IDM_PASSPHRASEMENU + MAX_CONFIGS) ) {
        ShowChangePassphraseDialog(&o.conn[LOWORD(wParam) - IDM_PASSPHRASEMENU]);
      }
#endif
      if (LOWORD(wParam) == IDM_IMPORT) {
        ImportConfigFile();
      }
      if (LOWORD(wParam) == IDM_SETTINGS) {
        ShowSettingsDialog();
      }
      if (LOWORD(wParam) == IDM_CLOSE) {
        CloseApplication(hwnd);
      }
      if (LOWORD(wParam) == IDM_SERVICE_START) {
        MyStartService();
      }
      if (LOWORD(wParam) == IDM_SERVICE_STOP) {
        MyStopService();
      }
      if (LOWORD(wParam) == IDM_SERVICE_RESTART) MyReStartService();
      break;

    case WM_CLOSE:
      CloseApplication(hwnd);
      break;

    case WM_DESTROY:
      WTSUnRegisterSessionNotification(hwnd);
      StopAllOpenVPN();
      OnDestroyTray();          /* Remove Tray Icon and destroy menus */
      PostQuitMessage (0);	/* Send a WM_QUIT to the message queue */
      break;

    case WM_QUERYENDSESSION:
      return(TRUE);

    case WM_ENDSESSION:
      StopAllOpenVPN();
      OnDestroyTray();
      break;

    case WM_WTSSESSION_CHANGE:
      switch (wParam) {
        case WTS_SESSION_LOCK:
          o.session_locked = TRUE;
          break;
        case WTS_SESSION_UNLOCK:
          o.session_locked = FALSE;
          if (CountConnState(suspended) != 0)
            ResumeConnections();
          break;
      }
      break;

    default:			/* for messages that we don't deal with */
      if (message == s_uTaskbarRestart)
        {
          /* Explorer has restarted, re-register the tray icon. */
          ShowTrayIcon();
          CheckAndSetTrayIcon();
          break;
        }
      return DefWindowProc (hwnd, message, wParam, lParam);
  }

  return 0;
}


static INT_PTR CALLBACK
AboutDialogFunc(UNUSED HWND hDlg, UINT msg, UNUSED WPARAM wParam, LPARAM lParam)
{
  LPPSHNOTIFY psn;
  if (msg == WM_NOTIFY) {
    psn = (LPPSHNOTIFY) lParam;
    if (psn->hdr.code == (UINT) PSN_APPLY)
        return TRUE;
  }
  return FALSE;
}


static void
ShowSettingsDialog()
{
  PROPSHEETPAGE psp[4];
  int page_number = 0;

  /* General tab */
  psp[page_number].dwSize = sizeof(PROPSHEETPAGE);
  psp[page_number].dwFlags = PSP_DLGINDIRECT;
  psp[page_number].hInstance = o.hInstance;
  psp[page_number].pResource = LocalizedDialogResource(ID_DLG_GENERAL);
  psp[page_number].pfnDlgProc = GeneralSettingsDlgProc;
  psp[page_number].lParam = 0;
  psp[page_number].pfnCallback = NULL;
  ++page_number;

  /* Proxy tab */
  if (o.service_only == 0) {
    psp[page_number].dwSize = sizeof(PROPSHEETPAGE);
    psp[page_number].dwFlags = PSP_DLGINDIRECT;
    psp[page_number].hInstance = o.hInstance;
    psp[page_number].pResource = LocalizedDialogResource(ID_DLG_PROXY);
    psp[page_number].pfnDlgProc = ProxySettingsDialogFunc;
    psp[page_number].lParam = 0;
    psp[page_number].pfnCallback = NULL;
    ++page_number;
  }

  /* Advanced tab */
  psp[page_number].dwSize = sizeof(PROPSHEETPAGE);
  psp[page_number].dwFlags = PSP_DLGINDIRECT;
  psp[page_number].hInstance = o.hInstance;
  psp[page_number].pResource = LocalizedDialogResource(ID_DLG_ADVANCED);
  psp[page_number].pfnDlgProc = AdvancedSettingsDlgProc;
  psp[page_number].lParam = 0;
  psp[page_number].pfnCallback = NULL;
  ++page_number;

  /* About tab */
  psp[page_number].dwSize = sizeof(PROPSHEETPAGE);
  psp[page_number].dwFlags = PSP_DLGINDIRECT;
  psp[page_number].hInstance = o.hInstance;
  psp[page_number].pResource = LocalizedDialogResource(ID_DLG_ABOUT);
  psp[page_number].pfnDlgProc = AboutDialogFunc;
  psp[page_number].lParam = 0;
  psp[page_number].pfnCallback = NULL;
  ++page_number;

  PROPSHEETHEADER psh;
  psh.dwSize = sizeof(PROPSHEETHEADER);
  psh.dwFlags = PSH_USEHICON | PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
  psh.hwndParent = o.hWnd;
  psh.hInstance = o.hInstance;
  psh.hIcon = LoadLocalizedIcon(ID_ICO_APP);
  psh.pszCaption = LoadLocalizedString(IDS_SETTINGS_CAPTION);
  psh.nPages = page_number;
  psh.nStartPage = 0;
  psh.ppsp = (LPCPROPSHEETPAGE) &psp;
  psh.pfnCallback = NULL;

  PropertySheet(&psh);
}


void
CloseApplication(HWND hwnd)
{
    int i;

    if (o.service_state == service_connected
    && ShowLocalizedMsgEx(MB_YESNO, _T("Exit VPN"), IDS_NFO_SERVICE_ACTIVE_EXIT) == IDNO)
            return;

    for (i = 0; i < o.num_configs; i++)
    {
        if (o.conn[i].state == disconnected)
            continue;

        /* Ask for confirmation if still connected */
        if (ShowLocalizedMsgEx(MB_YESNO, _T("Exit VPN"), IDS_NFO_ACTIVE_CONN_EXIT) == IDNO)
            return;
    }

    DestroyWindow(hwnd);
}

void
ImportConfigFile()
{

    TCHAR filter[2*_countof(o.ext_string)+5];

    _sntprintf_0(filter, _T("*.%s%c*.%s%c"), o.ext_string, _T('\0'), o.ext_string, _T('\0'));

    OPENFILENAME fn;
    TCHAR source[MAX_PATH] = _T("");

    fn.lStructSize = sizeof(OPENFILENAME);
    fn.hwndOwner = NULL;
    fn.lpstrFilter = filter;
    fn.lpstrCustomFilter = NULL;
    fn.nFilterIndex = 1;
    fn.lpstrFile = source;
    fn.nMaxFile = MAX_PATH;
    fn.lpstrFileTitle = NULL;
    fn.lpstrInitialDir = NULL;
    fn.lpstrTitle = NULL;
    fn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
    fn.lpstrDefExt = NULL;

    if (GetOpenFileName(&fn))
    {

        TCHAR destination[MAX_PATH];
        PTCHAR fileName = source + fn.nFileOffset;

        _sntprintf_0(destination, _T("%s\\%s"), o.config_dir, fileName);

        destination[_tcslen(destination) - _tcslen(o.ext_string) - 1] = _T('\0');

        if (EnsureDirExists(destination))
        {

            _sntprintf_0(destination, _T("%s\\%s"), destination, fileName);

            if (!CopyFile(source, destination, TRUE))
            {
                if (GetLastError() == ERROR_FILE_EXISTS)
                {
                    fileName[_tcslen(fileName) - _tcslen(o.ext_string) - 1] = _T('\0');
                    ShowLocalizedMsg(IDS_ERR_IMPORT_EXISTS, fileName);
                    return;
                }
            }
            else
            {
                ShowLocalizedMsg(IDS_NFO_IMPORT_SUCCESS);
                return;
            }

        }

        ShowLocalizedMsg(IDS_ERR_IMPORT_FAILED, destination);

    }

}

#ifdef DEBUG
void PrintDebugMsg(TCHAR *msg)
{
  time_t log_time;
  struct tm *time_struct;
  TCHAR date[30];

  log_time = time(NULL);
  time_struct = localtime(&log_time);
  _sntprintf(date, _countof(date), _T("%d-%.2d-%.2d %.2d:%.2d:%.2d"),
                 time_struct->tm_year + 1900,
                 time_struct->tm_mon + 1,
                 time_struct->tm_mday,
                 time_struct->tm_hour,
                 time_struct->tm_min,
                 time_struct->tm_sec);

  _ftprintf(o.debug_fp, _T("%s %s\n"), date, msg);
  fflush(o.debug_fp);
}
#endif

#define PACKVERSION(major,minor) MAKELONG(minor,major)
DWORD GetDllVersion(LPCTSTR lpszDllName)
{
    HINSTANCE hinstDll;
    DWORD dwVersion = 0;

    /* For security purposes, LoadLibrary should be provided with a
       fully-qualified path to the DLL. The lpszDllName variable should be
       tested to ensure that it is a fully qualified path before it is used. */
    hinstDll = LoadLibrary(lpszDllName);

    if(hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll,
                          "DllGetVersion");

        /* Because some DLLs might not implement this function, you
        must test for it explicitly. Depending on the particular
        DLL, the lack of a DllGetVersion function can be a useful
        indicator of the version. */

        if(pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            hr = (*pDllGetVersion)(&dvi);

            if(SUCCEEDED(hr))
            {
               dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
            }
        }

        FreeLibrary(hinstDll);
    }
    return dwVersion;
}

void
MsgToEventLog(WORD type, wchar_t *format, ...)
{
    const wchar_t *msg[2];
    wchar_t buf[256];
    int size = _countof(buf);

    if (!o.event_log)
    {
        o.event_log = RegisterEventSource(NULL, TEXT(PACKAGE_NAME));
        if (!o.event_log)
            return;
    }

    va_list args;
    va_start(args, format);
    int nchar = vswprintf(buf, size-1, format, args);
    va_end(args);

    if (nchar == -1) return;

    buf[size - 1] = '\0';

    msg[0] = TEXT(PACKAGE_NAME);
    msg[1] = buf;
    ReportEventW(o.event_log, type, 0, 0, NULL, 2, 0, msg, NULL);
}

void
ErrorExit(int exit_code, const wchar_t *msg)
{
    if (msg)
        MessageBoxExW(NULL, msg, TEXT(PACKAGE_NAME),
                      MB_OK | MB_SETFOREGROUND|MB_ICONERROR, GetGUILanguage());
    if (o.hWnd)
    {
        StopAllOpenVPN();
        PostQuitMessage(exit_code);
    }
    else
    {
        exit(exit_code);
    }
}
