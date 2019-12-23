#include "logger.h"

/*
获得当前时间字符串
@param buffer [out]: 时间字符串
@return 空
*/
void get_local_time(char* buffer)
{
    time_t rawtime;
    struct tm* timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
            (timeinfo->tm_year+1900), timeinfo->tm_mon+1, timeinfo->tm_mday,
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

/*
获得文件大小
@param filename [in]: 文件名
@return 文件大小
*/
long get_file_size(char* filename)
{
    long length = 0;
    FILE *fp = NULL;

    fp = fopen(filename, "rb");
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
    }

    if (fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }

    return length;
}

/*
写入日志文件
@param filename [in]: 日志文件名
@param max_size [in]: 日志文件大小限制
@param buffer [in]: 日志内容
@param buf_size [in]: 日志内容大小
@return 空
*/
void write_log_file( char* buffer)
{
    long max_size=FILE_MAX_SIZE;
    if (LOG_FILE != NULL && buffer != NULL)
    {
        unsigned buf_size=strlen(buffer);
        // 文件超过最大限制, 删除
        long length = get_file_size(LOG_FILE);

        if (length > max_size)
        {
            unlink(LOG_FILE); // 删除文件
        }

        // 写日志
        {
            FILE *fp;
            fp = fopen(LOG_FILE, "at+");
            if (fp != NULL)
            {
                char now[32];
                memset(now, 0, sizeof(now));
                get_local_time(now);
                fwrite(now, strlen(now), 1, fp);
                fputs(" ", fp);
                fwrite(buffer, buf_size, 1, fp);
                fputs("\n", fp);

                fclose(fp);
                fp = NULL;
            }
        }
    }
}

