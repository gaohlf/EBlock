#ifndef EBLOCKREQUESTMANAGER_H
#define EBLOCKREQUESTMANAGER_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/errno.h>
#include <linux/vmalloc.h>
#include <linux/bio.h>
#include "../include/EBlockRequests.h"

//最大的挂起IO数设定为256
#define MAX_PENDING_REQUESTS_NUM  256
/***
 * 该结构伴随一个IO在kernel的全过程
 * BRequestInKernel 结构从make_request时产生
 * 产生后立即进入EBRequestQUEUE，等待被Fetch
 * 用户态fetch走后，该结构从EBRequestQueue出队，进入PendingMap中
 * 用户完成request后，该从PendingMap中移除，并销毁。
 * *******************************/
struct EBRequestInKernel
{
    struct EBRequest req;
    struct bio *  bi;
    struct list_head list; //用于队列
    struct hlist_node hashnode; //用于hash表
};

//产生一个内核内部的IO，并挂入内部的请求队列
extern int generateEnqueueEBRequest(const char *name, struct bio * req);
//只销毁内存
extern void destroyEBRequest(struct EBRequestInKernel * requestInKernel);
//完成请求对应的bio并且销毁request
extern void completeEBRequest(struct EBRequestInKernel * requestInKernel, int err);
//打印EBRequestInKernel内容
extern void dumpEBRequestK(struct EBRequestInKernel * requestInKernel);


//预申请内存
extern int preAllocnEBRequests(void);

//销毁预申请内存
extern void destoryPreAllocEBRequests(void);

#endif