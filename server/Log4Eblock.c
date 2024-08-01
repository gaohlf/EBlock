#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "common/EColor.h"
#include "Log4Eblock.h"

typedef enum  {
    DEBUG_LEVEL = 0,
    INFO_LEVEL,
    WARNING_LEVEL,
    ERROR_LEVEL,
}log_level_t;

log_level_t logLevel = INFO_LEVEL;

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
    fprintf(stdout, "%s[%s]%s ",YELLOW_START,time_str, COLOE_RESET);
    vfprintf(stdout, format, args);
    fprintf(stdout, " %s%s:%d%s" ,BLUELINE_START, file, line,COLOE_RESET);
//    fprintf(stdout, " %s:%d [%s]: " , file, line, func);
    fprintf(stdout, "\n");
    va_end(args);
}

// 函数定义：格式化字符串
void eError(const char *file, int line, const char *func, const char *format, ...) {
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
    fprintf(stdout, "%s[%s]%s ",YELLOW_START,time_str, COLOE_RESET);
    fprintf(stdout, "%sERROR%s ",RED_START, COLOE_RESET);
    vfprintf(stdout, format, args);
    fprintf(stdout, " %s%s:%d%s" ,BLUELINE_START, file, line,COLOE_RESET);
//    fprintf(stdout, " %s:%d [%s]: " , file, line, func);
    fprintf(stdout, "\n");
    va_end(args);
}


// 函数定义：格式化字符串
void eWarning(const char *file, int line, const char *func, const char *format, ...) {

    // 获取当前时间
    time_t rawtime;
    struct tm *timeinfo;
    char time_str[20];
    if(logLevel > WARNING_LEVEL) return;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);

    // 格式化日志消息
    va_list args;
    va_start(args, format);
    fprintf(stdout, "%s[%s]%s ",YELLOW_START,time_str, COLOE_RESET);
    fprintf(stdout, "%sWARN%s ",BLUE_START, COLOE_RESET);
    vfprintf(stdout, format, args);
    fprintf(stdout, " %s%s:%d%s" ,BLUELINE_START, file, line,COLOE_RESET);
//    fprintf(stdout, " %s:%d [%s]: " , file, line, func);
    fprintf(stdout, "\n");
    va_end(args);
}

// 函数定义：格式化字符串
void eDebug(const char *file, int line, const char *func, const char *format, ...) {
    // 获取当前时间
    time_t rawtime;
    struct tm *timeinfo;
    char time_str[20];
    if(logLevel > DEBUG_LEVEL) return;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);

    // 格式化日志消息
    va_list args;
    va_start(args, format);
    fprintf(stdout, "%s[%s]%s ",YELLOW_START,time_str, COLOE_RESET);
    fprintf(stdout, "%sDEBUG%s ",GREEN_START, COLOE_RESET);
    vfprintf(stdout, format, args);
    fprintf(stdout, " %s%s:%d%s" ,BLUELINE_START, file, line,COLOE_RESET);
//    fprintf(stdout, " %s:%d [%s]: " , file, line, func);
    fprintf(stdout, "\n");
    va_end(args);
}


// 函数定义：格式化字符串
void eInfo(const char *file, int line, const char *func, const char *format, ...) {
    // 获取当前时间
    time_t rawtime;
    struct tm *timeinfo;
    char time_str[20];
    if(logLevel > INFO_LEVEL) return;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);

    // 格式化日志消息
    va_list args;
    va_start(args, format);
    fprintf(stdout, "%s[%s]%s ",YELLOW_START,time_str, COLOE_RESET);
    fprintf(stdout, "%sINFO%s ",CYAN_START, COLOE_RESET);
    vfprintf(stdout, format, args);
    fprintf(stdout, " %s%s:%d%s" ,BLUELINE_START, file, line,COLOE_RESET);
//    fprintf(stdout, " %s:%d [%s]: " , file, line, func);
    fprintf(stdout, "\n");
    va_end(args);
}