#ifndef EBLOCKPENDINGMAP_H
#define EBLOCKPENDINGMAP_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/hashtable.h>
#include <linux/dcache.h> // 包含full_name_hash的头文件
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include "EBlockRequestManager.h"

//从等待队列中取出后，进入pendingmap中的函数
extern void startPendingRequest(struct EBRequestInKernel * requestInKernel);
//根据kernelID查询pending的request
extern struct EBRequestInKernel * findPendingRequest(u64 kernelID);
//从pending队列中出并完成的函数
extern void endPendingRequest(u64 kernelID, int err);
//结束所有挂起EBrequest的生命周期 结束对于pending的request里的生命周期
extern void completeAllPendingEBRequests(int err);

//打印所有挂起EBrequest的
extern void dumpAllPendingEBRequests(void);

#endif