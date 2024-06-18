//
// Created by 20075 on 2024/6/4.
//

#include "EBlockIoctl.h"
#include "../include/EBLockDeviceCtrl.h"
#include "EblockErrors.h"
#include "Log4Eblock.h"

//取回一批request函数
int fetchRequests(struct EBRequests *requests)
{
    int fd, err;
//    ELOG("fetchRequests() before open\n");
    fd = open(CONTROL_PATH, O_RDWR);
    if(fd < 0) {
        ELOG("Failed to open device\n");
        return -EBLOCK_ERROR_TYPE_OPEN_FAILD;
    }

//    ELOG("fetchRequests() before ioctl p = %ld, \n", requests);
    err = ioctl(fd, EBLOCK_IOCTL_TYPE_FETCH_REQUESTS, requests);
    close(fd);
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

    fd = open(CONTROL_PATH, O_RDWR);
    if(fd < 0) {
        ELOG("Failed to open device\n");
        return -EBLOCK_ERROR_TYPE_OPEN_FAILD;
    }

    err = ioctl(fd, EBLOCK_IOCTL_TYPE_REQUEST_DONE, ctx);
    if(err != 0)
    {
        return -EBLOCK_ERROR_TYPE_IOCTL_EXE;
    }

    close(fd);
    return EBLOCK_ERROR_TYPE_OK;
}
