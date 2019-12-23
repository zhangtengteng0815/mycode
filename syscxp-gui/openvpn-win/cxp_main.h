#include "windows.h"
#include <stdbool.h>
#include <winsock.h>
#include <process.h>
//#include "syscxp.h"
#include "client.h"
#include "logger.h"

int cxp_start();
//void cxp_main();
DWORD WINAPI cxp_main(LPVOID lpParam);

void cxp_echo();