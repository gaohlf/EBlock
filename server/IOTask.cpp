//
// Created by 20075 on 2024/6/18.
//
#include <iostream>
#include <cstring>
#include <vector>
#include <chrono>
#include <string>
#include <cstdlib>
#include <cassert>
#include "IOTask.h"
#include "Log4Eblock.h"
#include "EblockErrors.h"
#include "EBlockDevices.h"

#define NSECTORS  2097152L
#define VDISKSIZE NSECTORS * 512
//static char *disk_buf = (char *) malloc(VDISKSIZE);
extern EBlockDevices devices;

IOTask::IOTask(struct EBRequest *req)
{
    data_buf = new char [req->length];
    assert(data_buf != nullptr);

    memcpy(&this->request, req, sizeof (struct  EBRequest));
}

void IOTask::process(void) {

    ELOG_INFO("devName : %s,"
         "isWrite : %d,"
         "length  : %llu,"
         "off     : %llu,"
         "kernelID: %llu,",
         request.devName, request.isWrite, request.length, request.off, request.kernelID);

    struct EBRequestDoneCtx ctx;
    ctx.kernelID = request.kernelID;
    if(request.isWrite)
    {
        ctx.err = processWrite();
        if(ctx.err)
        {
            ELOG("IOTask::process processWrite ctx.err %d\n", ctx.err);
        }
    } else
    {
        ctx.err = processRead();
        if(ctx.err)
        {
            ELOG("IOTask::process processRead ctx.err %d\n", ctx.err);
        }
    }
    ELOG("IOTask::process ctx.kernelID = %llu ctx.err %d\n",ctx.kernelID, ctx.err);
    int ret = requestDone(&ctx);
//            ELOG("requestDone success %d\n", ret);
    if(ret != EBLOCK_ERROR_TYPE_OK)
    {
        ELOG("requestDone failed %d\n", ret);
    }
    delete this;
}

IOTask::~IOTask() {
    if(data_buf != nullptr)
    {
        delete [] static_cast<char*>(data_buf);
    }
}


//处理读请求 从S3中读取，写入内核
int IOTask::processRead(void) {
    ELOG_INFO("devName : %s,"
         "isWrite : %d,"
         "length  : %llu,"
         "off     : %llu,"
         "kernelID: %llu,",
         request.devName, request.isWrite, request.length, request.off, request.kernelID);

    //从S3读取数据
    EBloclVirtualDevice *device =  devices.deviceByName(std::string(request.devName));
    memset(data_buf, 0, request.length);
    int ret = device->doRequestSync(&request, data_buf);
//    memcpy(data_buf, disk_buf + request.off,request.length);

    if(ret != EBLOCK_ERROR_TYPE_OK)
    {
        ELOG("IOTask::processRead KernelID = %llu ret= %d",request.kernelID, ret);
        return ret;
    }

    ELOG("IOTask::processRead KernelID = %llu",request.kernelID);

    //将databuf中的内容写入内核
    ret = writeToReadBio(&request, data_buf);

    ELOG_INFO("KernelID = %llu ret= %d",request.kernelID, ret);

    if(ret != EBLOCK_ERROR_TYPE_OK)
    {
        ELOG_ERROR("IOTask::processRead KernelID = %llu ret= %d",request.kernelID, ret);
    }
    return ret;
}

//处理写请求 从内核读取 写入S3
int IOTask::processWrite(void) {
    int ret = readFromWriteBio(&request, data_buf);
    if(ret != EBLOCK_ERROR_TYPE_OK)
    {
        ELOG("IOTask::processWrite KernelID = %llu ret= %d",request.kernelID, ret);
        return ret;
    }

    ELOG_INFO("devName : %s,"
         "isWrite : %d,"
         "length  : %llu,"
         "off     : %llu,"
         "kernelID: %llu,",
         request.devName, request.isWrite, request.length, request.off, request.kernelID);

    //写入S3
    EBloclVirtualDevice *device =  devices.deviceByName(std::string(request.devName));
    ret = device->doRequestSync(&request, data_buf);
    ELOG_INFO("KernelID = %llu ret= %d",request.kernelID, ret);
//    memcpy(disk_buf + request.off, data_buf, request.length);
    return ret;
}
