//
// Created by 20075 on 2024/6/4.
//

#ifndef EBLOCK_EBLOCKIOCTL_H
#define EBLOCK_EBLOCKIOCTL_H

#include "common/EBlockTypes.h"

#include "../include/EBLockDeviceCtrl.h"

#ifdef __cplusplus
extern "C" {
#endif

int fetchRequests(struct EBRequests *requests);

int requestDone(struct EBRequestDoneCtx *ctx);

//将读取结果写入到读请求中
int writeToReadBio(struct EBRequest *request, void *src_buf);

//从写请求中读取数据
int readFromWriteBio(struct EBRequest *request, void *dest_buf);

#ifdef __cplusplus
}
#endif



#endif //EBLOCK_EBLOCKIOCTL_H
