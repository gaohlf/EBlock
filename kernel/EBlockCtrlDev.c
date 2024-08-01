#include <linux/init.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
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
static ssize_t dev_write(struct file *filp, const char __user *buffer, size_t len, loff_t *offset);
static ssize_t dev_read(struct file *filp, char __user *buffer, size_t len, loff_t *offset);

static struct file_operations fops =
{
        .open = devOpen,
        .release = devRelease,
        .unlocked_ioctl = devIoctl,
        .read = dev_read,
        .write = dev_write,
};

//从用户态拷贝数据到内核态 返回错误码 0 表示拷贝成功
static int copyToEBReq(const char __user * srcBuffer, size_t srcLen, struct EBRequestInKernel *destReq)
{
    struct bio *bio = destReq->bi;
    struct bio_vec *bvec;
    void *kbuf;
    ssize_t total_bytes = 0;
    ssize_t bytes_to_copy;
    int i;
    if(srcLen != destReq->req.length)
    {
        printk(KERN_ERR "EBlockCtrlDev:copyFromEBReq kernelID %llu srcLen(%lu) != srcReq->req.length(%llu) failed\n",
            destReq->req.kernelID, srcLen, destReq->req.length);
        return -EFAULT;
    }

    // 遍历 bio 中的每个段
    bio_for_each_segment(bvec, bio, i) {
        kbuf = kmap(bvec->bv_page) + bvec->bv_offset;
        bytes_to_copy = min_t(ssize_t, bvec->bv_len, srcLen - total_bytes);

        if (copy_from_user(kbuf, srcBuffer + total_bytes, bytes_to_copy)) {
            kunmap(bvec->bv_page);
            return -EFAULT;
        }

        kunmap(bvec->bv_page);
        total_bytes += bytes_to_copy;

        if (total_bytes >= srcLen) break;
    }
    return 0;
}

//从用户态拷贝数据到内核态 返回错误码 0 表示拷贝成功
static int copyFromEBReq(struct EBRequestInKernel *srcReq, char __user *destBuffer, size_t destLen)
{
    struct bio *bio = srcReq->bi;
    struct bio_vec *bvec;
    void *kbuf;
    ssize_t total_bytes = 0;
    ssize_t bytes_to_copy;
    int i;
    if(destLen != srcReq->req.length)
    {
        printk(KERN_ERR "EBlockCtrlDev:copyFromEBReq kernelID %llu destLen(%lu) != srcReq->req.length(%llu) failed\n",
            srcReq->req.kernelID, destLen, srcReq->req.length);
        return -EFAULT;
    }

    // 遍历 bio 中的每个段
    bio_for_each_segment(bvec, bio, i) {
        kbuf = kmap(bvec->bv_page) + bvec->bv_offset;
        bytes_to_copy = min_t(ssize_t, bvec->bv_len, destLen - total_bytes);

        if (copy_to_user(destBuffer + total_bytes, kbuf, bytes_to_copy)) {
            kunmap(bvec->bv_page);
            return -EFAULT;
        }

        kunmap(bvec->bv_page);
        total_bytes += bytes_to_copy;

        if (total_bytes >= destLen) break;
    }
    return 0;
}

//正确是返回长度和传入长度相同
static ssize_t dev_write(struct file *filp, const char __user *buffer, size_t len, loff_t *offset)
{
    ssize_t bytes_written = len;
    u64 kernelID = *offset;
    struct EBRequestInKernel * ebReq;
    int err = 0;
    printk(KERN_INFO "EBlockCtrlDev:dev_write kernelID %llu\n", kernelID);
    //FIXME:在数据流关键路径加桩点
    ebReq = findPendingRequest(kernelID);
    if(ebReq == NULL)
    {
        printk(KERN_ERR "EBlockCtrlDev:dev_write  kernelID %llu findPendingRequest failed\n", kernelID);
        return -EFAULT;
    }
    //FIXME:在数据流关键路径加桩点
    err = copyToEBReq(buffer, len, ebReq);
    if(err != 0)
    {
        printk(KERN_ERR "EBlockCtrlDev:dev_write kernelID %llu copyToEBReq failed err = %d\n", kernelID, err);
        return err;
    }
    return bytes_written;
}

//正确是返回长度和传入长度相同
static ssize_t dev_read(struct file *filp, char __user *buffer, size_t len, loff_t *offset)
{
    ssize_t bytes_read = len;
    u64 kernelID = *offset;
    struct EBRequestInKernel * ebReq;
    int err = 0;
    printk(KERN_INFO "EBlockCtrlDev:dev_read kernelID %llu\n", kernelID);

    //FIXME:在数据流关键路径加桩点
    ebReq = findPendingRequest(kernelID);
    if(ebReq == NULL)
    {
        printk(KERN_ERR "EBlockCtrlDev:dev_read kernelID %llu findPendingRequest failed\n", kernelID);
        return -EFAULT;
    }
    //FIXME:在数据流关键路径加桩点
    err = copyFromEBReq(ebReq, buffer, len);
    if(err != 0)
    {
        printk(KERN_ERR "EBlockCtrlDev:dev_write kernelID %llu copyFromEBReq failed err = %d\n", kernelID, err);
        return err;
    }
    return bytes_read;
}

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
        //FIXME:在数据流关键路径加桩点
        startPendingRequest(requestInKernel);
        //FIXME:在数据流关键路径加桩点
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
    printk(KERN_ERR "**********EBlockCtrlDev::Requestdone after copy_from_user err = %lu\n", err);
    if(err != 0)
    {
        printk(KERN_ERR "EBlockCtrlDev::Requestdone failed\n");
        return;
    }

    printk(KERN_ERR "**********EBlockCtrlDev::Requestdone reqDone.err = %d\n", reqDone.err);
    if(reqDone.err != 0)
    {
        reqDone.err = -EIO;
    }

    //FIXME:在数据流关键路径加桩点
    endPendingRequest(reqDone.kernelID, reqDone.err);
    //FIXME:在数据流关键路径加桩点
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
