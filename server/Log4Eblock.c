#include <stdio.h>
#include <stdarg.h>
#include <time.h>


// 函数定义：格式化字符串
void eLogMessage(const char *file, int line, const char *func, const char *format, ...) {
    // 获取当前时间
    time_t rawtime;
    struct tm *timeinfo;
    char time_str[20];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);

    // 格式化日志消息
    va_list args;
    va_start(args, format);
    fprintf(stdout, "[%s] %s:%d (%s): ", time_str, file, line, func);
    vfprintf(stdout, format, args);
    fprintf(stdout, "\n");
    va_end(args);
}