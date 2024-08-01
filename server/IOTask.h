//
// Created by 20075 on 2024/6/18.
//

#ifndef EBLOCK_IOTASK_H
#define EBLOCK_IOTASK_H

#include "EBlockIoctl.h"
class IOTask {
private:
    //该request会贯穿TASK的生命周期其他函数科放心使用request
    struct EBRequest request;
    void * data_buf = nullptr;//数据空间
private:
    //处理读请求 从S3中读取，写入内核
    int processRead(void);
    //处理写请求 从内核读取 写入S3
    int processWrite(void);
public:
    IOTask(struct EBRequest *req);
    ~IOTask();

    //通过值传递request
    void process(void);
};


#endif //EBLOCK_IOTASK_H
