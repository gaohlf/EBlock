#include "EBlockRequestManager.h"
#include "EBlockRequestQueue.h"
#include "EBlockStatics.h"
#include <linux/bio.h>

static u64 globalRequestIDCounter = 1;
static DEFINE_SPINLOCK(idSpinlock);

//获取一个非零自加的ID
static u64 newRequestID(void )
{
    u64 id = 0;
    spin_lock(&idSpinlock);   // 获取锁，并保存当前中断状态
    if(++globalRequestIDCounter == 0)
        globalRequestIDCounter = 1;
    id = globalRequestIDCounter;
    // 访问共享资源
    spin_unlock(&idSpinlock);  // 释放锁，并恢复之前的中断状态
    return id;
}

/**
 * 以下四个函数是用于内部管理EBRequestInKernel的内存生存空间的
 * *****/
/**在模块加载时预先申请有可能用到的EBRequest***/

//用于内存管理的锁
static DEFINE_SPINLOCK(memSpinlock);
//用于申请和销毁销毁
static struct EBRequestInKernel * ebAllRequestInKernels[MAX_PENDING_REQUESTS_NUM];
//管理内存分配
static struct EBRequestInKernel * freeRequestInKernels[MAX_PENDING_REQUESTS_NUM];

//pos范围是 -1 ，到MAX_PENDING_REQUESTS_NUM-1， pos  为-1代表没有内存
static int pos = MAX_PENDING_REQUESTS_NUM-1;

//预申请内存
int preAllocnEBRequests(void)
{
    int i = 0, free_i = 0;
    for (i = 0; i < MAX_PENDING_REQUESTS_NUM; ++i) {
        ebAllRequestInKernels[i] = kmalloc(sizeof(struct EBRequestInKernel), GFP_KERNEL);
        if( ebAllRequestInKernels[i] == NULL)
        {
            break;
        }
        freeRequestInKernels[i] = ebAllRequestInKernels[i];
    }
    //所有内存申请完毕
    if(i != MAX_PENDING_REQUESTS_NUM - 1)
        return 0;

    //释放已申请的内存
    for(free_i = 0; free_i < i; ++free_i)
    {
        kfree(ebAllRequestInKernels[i]);
    }
    return -ENOMEM;
}

//销毁预申请内存
void destoryPreAllocEBRequests(void)
{
    int i = 0;
    for (i = 0; i < MAX_PENDING_REQUESTS_NUM; ++i) {
        kfree(ebAllRequestInKernels[i]);
    }
}

//申请一个EBRequest空间
static struct EBRequestInKernel * newEBRequest(void)
{
    struct EBRequestInKernel  * requestInKernel = NULL;
    spin_lock(&memSpinlock);   // 获取
    if(pos < 0)
    {
        spin_unlock(&memSpinlock);
        return NULL;
    }
    requestInKernel = freeRequestInKernels[pos--];

    spin_unlock(&memSpinlock);   // 获取
    //分配空间 清空数据
    requestInKernel->bi = NULL;
    requestInKernel->req.isWrite = 0;
    requestInKernel->req.off = 0;
    requestInKernel->req.length = 0;
    memset(requestInKernel->req.devName, 0 , BLOCK_CONTEXT_NAME_SIZE);
    requestInKernel->req.kernelID = 0;
    INIT_LIST_HEAD(&requestInKernel->list);
    INIT_HLIST_NODE(&requestInKernel->hashnode);
    return requestInKernel;
}

//销毁EBrequest
void destroyEBRequest(struct EBRequestInKernel * requestInKernel)
{
    spin_lock(&memSpinlock);   // 获取
    freeRequestInKernels[++pos] = requestInKernel;
    spin_unlock(&memSpinlock);   // 获取
}
/**********************************************************/


//NOTE:构造一个request并入对 已放弃使用
//int generateEnqueueEBRequestReq(const char *name, struct request * req)
//{
//    struct EBRequestInKernel * requestInKernel;
//    if(strlen(name) >= BLOCK_CONTEXT_NAME_SIZE)
//    {
//        printk(KERN_ERR "EBlockRequestManager::allocEBRequest too long name!\n");
//        return -ENOMEM;
//    }
//
//    requestInKernel = newEBRequest();
//    if(!requestInKernel)
//    {
//        printk(KERN_ERR "EBlockRequestManager::allocEBRequest kmalloc faild!\n");
//        return -ENOMEM;
//    }
//    requestInKernel->req_data = req;
//
//    //填充io相关变量
//    requestInKernel->req.isWrite = (rq_data_dir(req) == REQ_OP_WRITE);
//    requestInKernel->req.off = blk_rq_pos(req) * SECTOR_SIZE;
//    requestInKernel->req.length = blk_rq_bytes(req);
//    //复制名称 代表复制块设备
//    memset(requestInKernel->req.devName, 0 , BLOCK_CONTEXT_NAME_SIZE);
//    strncpy(requestInKernel->req.devName, name, strlen(name));
//    //生成唯一ID
//    requestInKernel->req.kernelID = newRequestID();
//    //将EBRequestInKernel 插入等待唤醒队列
//    enRequestQueue(requestInKernel);
//    return 0;
//}


//构造一个request并入对
int generateEnqueueEBRequest(const char *name, struct bio * bi)
{
    struct EBRequestInKernel * requestInKernel;
    if(strlen(name) >= BLOCK_CONTEXT_NAME_SIZE)
    {
        printk(KERN_ERR "EBlockRequestManager::allocEBRequest too long name!\n");
        return -ENOMEM;
    }

    requestInKernel = newEBRequest();
    if(!requestInKernel)
    {
        printk(KERN_ERR "EBlockRequestManager::allocEBRequest kmalloc faild!\n");
        return -ENOMEM;
    }
    requestInKernel->bi = bi;

    //填充io相关变量
    requestInKernel->req.isWrite = (bio_data_dir(bi) == WRITE);
    requestInKernel->req.off = bi->bi_sector * SECTOR_SIZE;
    requestInKernel->req.length = bi->bi_size;
    //复制名称 代表复制块设备
    memset(requestInKernel->req.devName, 0 , BLOCK_CONTEXT_NAME_SIZE);
    strncpy(requestInKernel->req.devName, name, strlen(name));
    //生成唯一ID
    requestInKernel->req.kernelID = newRequestID();
    //FIXME:在数据流关键路径加桩点
    //将EBRequestInKernel 插入等待唤醒队列
    enRequestQueue(requestInKernel);
    //FIXME:在数据流关键路径加桩点
    return 0;
}


void completeEBRequest(struct EBRequestInKernel * requestInKernel, int err)
{
    int status = 0;
//    struct bio *bio;
    //FIXME:在数据流关键路径加桩点
    printk(KERN_ERR "EBlockRequestManager::completeEBRequest bio = %p err = %d\n", requestInKernel->bi, err);

    if(err != 0)
    {//错误码转义
        status = -EIO;
    }
//
//    //遍历req中的所有BIO分别完成
//    __rq_for_each_bio(bio, requestInKernel->req_data) {
//        printk(KERN_INFO "EBlockRequestManager::completeEBRequest before bio_endio bio = %p status = %d\n",bio, status);
//        bio_endio(bio, status);
//        printk(KERN_INFO "EBlockRequestManager::completeEBRequest after bio_endio bio = %p status = %d\n",bio, status);
//    }
    printk(KERN_INFO "EBlockRequestManager::completeEBRequest before bio_endio bio = %p status = %d\n",
           requestInKernel->bi, status);
    bio_endio(requestInKernel->bi, status);
    printk(KERN_INFO "EBlockRequestManager::completeEBRequest after bio_endio bio = %p status = %d\n",
            requestInKernel->bi, status);

    //FIXME:在数据流关键路径加桩点
    ioComplete();
    destroyEBRequest(requestInKernel);
}

//打印一个数据结构
static void dumpEBRequest(struct EBRequest * req)
{
    printk(KERN_ERR "++++req      %p\n", req);
    printk(KERN_ERR "++++++devName  %s\n", req->devName);
    printk(KERN_ERR "++++++isWrite  %d\n", req->isWrite);
    printk(KERN_ERR "++++++length   %llu\n", req->length);
    printk(KERN_ERR "++++++off      %llu\n", req->off);
    printk(KERN_ERR "++++++kernelID %llu\n", req->kernelID);
}

//打印EBRequestInKernel内容
void dumpEBRequestK(struct EBRequestInKernel * requestInKernel)
{
    printk(KERN_ERR "++requestInKernel %p\n", requestInKernel);
    dumpEBRequest(&requestInKernel->req);
    printk(KERN_ERR "++++bi   %p\n", requestInKernel->bi);
}

EXPORT_SYMBOL(preAllocnEBRequests);
EXPORT_SYMBOL(destoryPreAllocEBRequests);

EXPORT_SYMBOL(generateEnqueueEBRequest);
EXPORT_SYMBOL(destroyEBRequest);
EXPORT_SYMBOL(completeEBRequest);
EXPORT_SYMBOL(dumpEBRequestK);
