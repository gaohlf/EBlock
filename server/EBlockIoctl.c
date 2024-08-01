//
// Created by 20075 on 2024/6/4.
//

#include "EBlockIoctl.h"
#include "../include/EBLockDeviceCtrl.h"
#include "EblockErrors.h"
#include "Log4Eblock.h"


int globalFd = - 1;

int tryOpen(void)
{
    if(globalFd < 0)
    {
        globalFd = open(CONTROL_PATH, O_RDWR);
    }
    return globalFd;
}

//取回一批request函数
int fetchRequests(struct EBRequests *requests)
{
    int fd, err;
//    ELOG("fetchRequests() before open\n");
    fd = tryOpen();
    if(fd < 0) {
        ELOG("Failed to open device\n");
        return -EBLOCK_ERROR_TYPE_OPEN_FAILD;
    }

//    ELOG("fetchRequests() before ioctl p = %ld, \n", requests);
    err = ioctl(fd, EBLOCK_IOCTL_TYPE_FETCH_REQUESTS, requests);
    if(err != 0)
    {
        ELOG("fetchRequests failed ioctl requests = %p, err = %d\n", requests, err);
        return -EBLOCK_ERROR_TYPE_IOCTL_EXE;
    }
//    ELOG("fetchRequests() after ioctl val = %p,err = %d requests.requestNum = %llu\n", requests, err, requests->requestNum);
    return err;
}

//取回一批request函数
int requestDone(struct EBRequestDoneCtx *ctx)
{
    int fd, err;

    fd = tryOpen();
    if(fd < 0) {
        ELOG("Failed to open device\n");
        return -EBLOCK_ERROR_TYPE_OPEN_FAILD;
    }

    err = ioctl(fd, EBLOCK_IOCTL_TYPE_REQUEST_DONE, ctx);
    if(err != 0)
    {
        return -EBLOCK_ERROR_TYPE_IOCTL_EXE;
    }
    return EBLOCK_ERROR_TYPE_OK;
}

//将读取结果写入到读请求中
int writeToReadBio(struct EBRequest *request, void *src_buf)
{
    int fd;
    ssize_t bytes_written ;
    fd = tryOpen();
    if(fd < 0) {
        ELOG("Failed to open device\n");
        return -EBLOCK_ERROR_TYPE_OPEN_FAILD;
    }

    bytes_written = pwrite(fd, src_buf, request->length, request->kernelID);
    if (bytes_written != request->length)
    {
        return -EBLOCK_ERROR_TYPE_WRITE_FAILED;
    }
    return EBLOCK_ERROR_TYPE_OK;
}

//从写请求中读取数据
int readFromWriteBio(struct EBRequest *request, void *dest_buf)
{
    int fd;
    ssize_t bytes_read;
    fd = tryOpen();
    if(fd < 0) {
        ELOG("Failed to open device\n");
        return -EBLOCK_ERROR_TYPE_OPEN_FAILD;
    }

    bytes_read = pread(fd, dest_buf, request->length, request->kernelID);
    if (bytes_read != request->length)
    {
        return -EBLOCK_ERROR_TYPE_READ_FAILED;
    }

    return EBLOCK_ERROR_TYPE_OK;
}
