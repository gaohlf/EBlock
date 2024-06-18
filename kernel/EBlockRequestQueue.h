#ifndef EBLOCKREQUESTQUEUE_H
#define EBLOCKREQUESTQUEUE_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include "EBlockRequestManager.h"

/** 判断等待被取走的队列是否为空 ***/
extern bool requestQueueEmpty(void);

//将事先申请的入队
extern void enRequestQueue(struct EBRequestInKernel *requestInKernel);

//将request列表取出到head中，一次至多取出MAX_SWAP_REQUESTS_ONCE个请求
extern int fetchRequestQueueOnce(struct list_head *out_list);

extern struct EBRequestInKernel *requestByListHead(struct list_head * pos);

extern void completeAllQueueRequests(int err);

//打印队列信息
extern void dumpAllQueueRequests(void);
#endif