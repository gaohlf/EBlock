//
// Created by 20075 on 2024/6/12.
//

#ifndef EBLOCK_LOG4EBLOCK_H
#define EBLOCK_LOG4EBLOCK_H

#ifdef __cplusplus
extern "C" {
#endif


void eLogMessage(const char *file, int line, const char *func, const char *format, ...);

#ifdef __cplusplus
}
#endif

//FIXME：兼容内核和用户态，增加日志界别
#define ELOG(format, ...) \
    eLogMessage(__FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#endif //EBLOCK_LOG4EBLOCK_H