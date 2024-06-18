//
// Created by 20075 on 2024/6/4.
//

#ifndef EBLOCK_EBLOCKDEVICECTRL_H
#define EBLOCK_EBLOCKDEVICECTRL_H

#include "../include/EBlockRequests.h"



#define CONTROL_CLASS_NAME "eblock_class"
#define CONTROL_NAME  "EblockCtrl"
#define CONTROL_PATH   "/dev/" CONTROL_NAME

#ifdef __cplusplus
extern "C" {
#endif

//党用户态完成IO时
struct __attribute__((packed)) EBRequestDoneCtx
{
    u64   kernelID;
    int   err;
};

#ifdef __cplusplus
}
#endif

/***取requests 取回结构为 EBRequests，一次至多取回MAX_SWAP_REQUESTS_ONCE个requests *****/
#define EBLOCK_IOCTL_TYPE_FETCH_REQUESTS _IOR('a', 1, struct EBRequests)
//当一个写IO完成  给后台的回调
#define EBLOCK_IOCTL_TYPE_REQUEST_DONE   _IOW('a', 1, struct EBRequestDoneCtx)

#endif //EBLOCK_EBLOCKDEVICECTRL_H
