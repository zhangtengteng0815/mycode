#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "metadata.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>		// linux下头文件
#endif

#define FILE_MAX_SIZE (1024*1024)

char LOG_FILE[100];
/*
写入日志文件
@param filename [in]: 日志文件名
@param max_size [in]: 日志文件大小限制
@param buffer [in]: 日志内容
@param buf_size [in]: 日志内容大小
@return 空
*/
void write_log_file(char* buffer);
