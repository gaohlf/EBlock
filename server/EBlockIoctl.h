//
// Created by 20075 on 2024/6/4.
//

#ifndef EBLOCK_EBLOCKIOCTL_H
#define EBLOCK_EBLOCKIOCTL_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>

typedef uint8_t u8;
typedef uint64_t u64;

#include "../include/EBLockDeviceCtrl.h"

#ifdef __cplusplus
extern "C" {
#endif

int fetchRequests(struct EBRequests *requests);

int requestDone(struct EBRequestDoneCtx *ctx);

#ifdef __cplusplus
}
#endif



#endif //EBLOCK_EBLOCKIOCTL_H
