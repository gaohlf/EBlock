#include <linux/init.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include "../include/EBLockDeviceCtrl.h"
#include "EBlockRequestQueue.h"
#include "EBlockPendingMap.h"
#include "EBlockRequestManager.h"
#include "EBLockGlobalCond.h"
#include "EBlockStatics.h"

static int majorNumber;
static struct class* excharClass = NULL;
static struct device* excharDevice = NULL;

static int devOpen(struct inode *, struct file *);
static int devRelease(struct inode *, struct file *);
static long devIoctl(struct file *, unsigned int, unsigned long);

static struct file_operations fops =
        {
                .open = devOpen,
                .release = devRelease,
                .unlocked_ioctl = devIoctl,
        };


static int devOpen(struct inode *inodep, struct file *filep){
    return 0;
}

static int devRelease(struct inode *inodep, struct file *filep){
    return 0;
}

static void translateListhead2EBRequests(struct EBRequests * requests, struct list_head *head)
{
    struct EBRequestInKernel *requestInKernel, *tmp;
    //遍历请求取回列表
    list_for_each_entry_safe(requestInKernel, tmp, head, list)
    {
        memcpy(&requests->requests[requests->requestNum++], &requestInKernel->req, sizeof(struct EBRequest));
        startPendingRequest(requestInKernel);
    }
}


//没有拷贝的数据长度
static unsigned long tryFetchRequestsToUser(unsigned long arg)
{
    LIST_HEAD(head);
    struct EBRequests ebRequests;
    int count = fetchRequestQueueOnce(&head);
    //若取出出错则 保持bRequests.requestNum 为0
    ebRequests.requestNum = 0;

    if(count > 0)
    {
        printk(KERN_INFO "EBlockCtrlDev::fetchRequestsToUser %llu count = %d\n", ebRequests.requestNum, count);
        translateListhead2EBRequests(&ebRequests, &head);
    }
    else
    {//没有请求时直接报错
        return sizeof(ebRequests);
    }

    //超时返回 ebRequests 中 requestNum 为0
    return copy_to_user((struct EBRequests*) arg, &ebRequests, sizeof(struct EBRequests));
}

//取出请求并拷贝到用户态
static unsigned long fetchRequestsToUser(unsigned long arg)
{
    unsigned long len_not_copied = tryFetchRequestsToUser(arg);
    if(len_not_copied == 0)
    {
        return len_not_copied;
    }
    //若没有取到，等待一个超时唤醒时间再重试
    waitEBlockCond();
    return tryFetchRequestsToUser(arg);
}

//取出请求并拷贝到用户态
static void Requestdone(unsigned long arg)
{
    struct EBRequestDoneCtx reqDone;
    unsigned long err = copy_from_user(&reqDone, (struct EBRequestDoneCtx*) arg,  sizeof(struct EBRequestDoneCtx));
    if(err != 0)
    {
        printk(KERN_ERR "EBlockCtrlDev::Requestdone failed\n");
        return;
    }

    if(reqDone.err != 0)
    {
        reqDone.err = -EIO;
    }

    endPendingRequest(reqDone.kernelID, reqDone.err);
}

static long devIoctl(struct file *filep, unsigned int cmd, unsigned long arg){

    switch(cmd){
        case EBLOCK_IOCTL_TYPE_FETCH_REQUESTS:
        {
            unsigned long len_not_copied = fetchRequestsToUser(arg);
            if(len_not_copied != 0 )
            {
                printk(KERN_ERR "EBlockCtrlDev::devIoctl copy failed len_not_copied = %lu arg = %lu\n",
                       len_not_copied, arg);
            }
            return 0;
        }
        case EBLOCK_IOCTL_TYPE_REQUEST_DONE:
        {
            ioDoneByUser();
            Requestdone(arg);
            return 0;
        }
    }
    printk(KERN_ERR "EBlockCtrlDev::devIoctl command not found\n");
    return 0;
}


int devCtrlInit(void){
    majorNumber = register_chrdev(0, CONTROL_NAME, &fops);
    excharClass = class_create(THIS_MODULE, CONTROL_CLASS_NAME);
    excharDevice = device_create(excharClass, NULL, MKDEV(majorNumber, 0), NULL, CONTROL_NAME);
    return 0;
}

void  devCtrlExit(void){
    device_destroy(excharClass, MKDEV(majorNumber, 0));
    class_unregister(excharClass);
    class_destroy(excharClass);
    unregister_chrdev(majorNumber, CONTROL_NAME);
}

EXPORT_SYMBOL(devCtrlInit);
EXPORT_SYMBOL(devCtrlExit);
