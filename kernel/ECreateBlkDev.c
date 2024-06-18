#include "ECreateBlkDev.h"
#include "EBlockContext.h"
#include "../include/EBlockRequests.h"
#include "EBlockRequestManager.h"
#include "EBlockRequestQueue.h"
#include "EBlockPendingMap.h"
#include "EBLockGlobalCond.h"
#include "EBlockStatics.h"

//FIXME 需要支持更多卷设备时 变为多段指定
#define EBLOCK_MAJOR 240
//FIXME 变为创建时指定
#define NSECTORS  2097152L //(1024*1024*2)Number of sectors in the device



//NOTE:根据file获取block device
struct block_device * blkdevByFile(struct file *file)
{
    struct block_device *dev =  file->private_data;
    return dev;
}

//根据Blockdev获取gendisk
struct gendisk * gendiskByBlkdev(struct block_device *bdev)
{
    struct gendisk *gd = bdev->bd_disk;
    return gd;
}


//根据gdisk得到上下文
struct EBlockContext *contextBygendisk(struct gendisk *gd)
{
    struct EBlockContext * context = gd->private_data;
    return context;
}

//根据request得到上下文
struct EBlockContext *contextByReq(struct request *req)
{
    struct gendisk *gd = req->rq_disk;
    if(!gd)
    {
        printk(KERN_ERR "ECreateBlkDev::contextByQueue faild no gd\n");
        return NULL;
    }
    return contextBygendisk(gd);
}

//根据bio得到上下文
struct EBlockContext *contextByBio(struct bio *bi)
{
    struct block_device *bdev = bi->bi_bdev;
    struct gendisk *gd = bdev->bd_disk;
    return contextBygendisk(gd);
}

//根据file获取context上下文
struct EBlockContext * contextByFile(struct file *file)
{
    struct block_device *dev = blkdevByFile(file);
    struct gendisk *gd = gendiskByBlkdev(dev);
    struct EBlockContext * context = gd->private_data;
    return context;
}

/**
 * 数据IO的关键函数
 * ************************************************************************/



//        //遍历vector
//        rq_for_each_segment(bv, req, iter) {
//            char *buffer = page_address(bv->bv_page) + bv->bv_offset;
//
////            if (is_write)
////                memcpy(data + sector_pos * SECTOR_SIZE, buffer, bv->bv_len);
////            else
////                memcpy(buffer, data + sector_pos * SECTOR_SIZE, bv->bv_len);
//
//            sector_pos += bv->bv_len / SECTOR_SIZE;
//        }
//static void eblock_make_request(struct request_queue *q)
//{
//    int ret = 0;
//    struct request *req;
//    printk(KERN_INFO "ECreateBlkDev::eblock_make_request IN q = %p\n", q);
//    while ((req = blk_fetch_request(q)) != NULL) {
//        struct EBlockContext * context = NULL;
//
//        if (req->cmd_flags & REQ_FLUSH || req->cmd_flags & REQ_FUA) {
//            // 如果是控制请求，直接完成请求并返回
//            __blk_end_request_all(req, 0);
//            continue;
//        }
//
//        ioStart();//取到一个IO
//        context = contextByReq(req);
//        if(context == NULL)
//        {
//            printk(KERN_ERR "ECreateBlkDev::eblock_make_request failed! context is null req = %p\n", req);
//            __blk_end_request_all(req, -EIO);
//            ioComplete();
//            continue;
//        }
//
//        //FIXME:异步改造
//        printk(KERN_INFO "ECreateBlkDev::eblock_make_request before generateEnqueueEBRequest req = %p\n", req);
//        //生成等待请求头，扔进等待队列
//        ret = generateEnqueueEBRequest(context->name, req);
//        printk(KERN_INFO "ECreateBlkDev::generateEnqueueEBRequest IN q = %p context->name = [%s] ret = %d req = %p\n",
//               q, context->name, ret, req);
//        if(ret != 0)
//        {
//            printk(KERN_ERR "ECreateBlkDev::eblock_make_request generateEnqueueEBRequest failed! req = %p ret = %d\n",
//                    req, ret);
//            __blk_end_request_all(req, ret);
//            ioComplete();
//        }
//        printk(KERN_INFO "ECreateBlkDev::eblock_make_request before wakeupEBlockCond req = %p\n", req);
//        //入队后唤醒等待fetch队列唤醒
//        wakeupEBlockCond();
//        printk(KERN_INFO "ECreateBlkDev::eblock_make_request after wakeupEBlockCond req = %p\n", req);
//    }
//    printk(KERN_INFO "ECreateBlkDev::eblock_make_request OUT = %p\n", q);
//}

//代替IO处理
static void eblockMakeRequest(struct request_queue *q, struct bio *bio)
{
    int ret = 0;
    struct EBlockContext * context = NULL;
    printk(KERN_INFO "ECreateBlkDev::eblockMakeRequest IN q = %p, bio = %p\n", q, bio);
    ioStart();//取到一个IO

    if (bio->bi_rw & REQ_DISCARD) {
        printk(KERN_INFO "Skipping control command for bio: %p\n",bio);
        bio_endio(bio, 0); // 完成 BIO 请求
        return;
    }

    context = contextByBio( bio);
    if(context == NULL)
    {
        printk(KERN_ERR "ECreateBlkDev::eblockMakeRequest failed! context is null bio = %p\n", bio);
        bio_endio(bio, -EIO);
        ioComplete();
        return;
    }


    printk(KERN_INFO "ECreateBlkDev::eblockMakeRequest before generateEnqueueEBRequest bio = %p\n", bio);
    //生成等待请求头，扔进等待队列
    ret = generateEnqueueEBRequest(context->name, bio);
    printk(KERN_INFO "ECreateBlkDev::eblockMakeRequest IN q = %p context->name = [%s] ret = %d bio = %p\n",
           q, context->name, ret, bio);
    if(ret != 0)
    {
        printk(KERN_ERR "ECreateBlkDev::eblockMakeRequest generateEnqueueEBRequest failed! bio = %p ret = %d\n",
                bio, ret);
        bio_endio(bio, ret);
        ioComplete();
    }
    printk(KERN_INFO "ECreateBlkDev::eblockMakeRequest before wakeupEBlockCond bio = %p\n", bio);
    //入队后唤醒等待fetch队列唤醒
    wakeupEBlockCond();
    printk(KERN_INFO "ECreateBlkDev::eblockMakeRequest after wakeupEBlockCond bio = %p\n", bio);
    printk(KERN_INFO "ECreateBlkDev::eblockMakeRequest OUT = %p bio = %p\n", q, bio);
}



//在打开设备时关联file和block
static int eblock_open(struct block_device *bdev, fmode_t mode)
{
    struct file *file = bdev->bd_disk->private_data;
    printk(KERN_INFO "Block device opened\n");
    file->private_data = bdev;
    return 0;
}

static void eblock_release(struct gendisk *gd, fmode_t mode)
{
    printk(KERN_INFO "Block device closed\n");
    //FIXME :
}

static struct block_device_operations   eblock_fops = {
        .owner = THIS_MODULE,
        .open = eblock_open,
        .release = eblock_release,
};

//创一个Eblock设备名字为name
int  createEblockDev(const char * name)
{
    struct EBlockContext *context = newEBlockContext(name);
    if(context == NULL)
    {
        printk(KERN_ERR "CreateDev:newEBlockContext failed -ENOMEM!\n");
        return -ENOMEM;
    }
//从处理q中req变为处理单个的bio函数
//    context->queue = blk_init_queue(eblock_make_request, NULL);
    context->queue = blk_alloc_queue(GFP_KERNEL);
    if (context->queue == NULL) {
        printk(KERN_ERR "CreateDev:blk_init_queue failed -ENOMEM!\n");
        removeEBlockContext(name);
        return -ENOMEM;
    }
    blk_queue_make_request(context->queue, eblockMakeRequest);
    blk_queue_logical_block_size(context->queue, SECTOR_SIZE);

    //NOTE 注册一个新的块设备编号
    context->gd = alloc_disk(1);
    if (!context->gd) {
        printk(KERN_ERR "CreateDev:alloc_disk failed -ENOMEM!\n");
        blk_cleanup_queue(context->queue);
        removeEBlockContext(name);
        return -ENOMEM;
    }

    //NOTE: major分类
    context->gd->major = EBLOCK_MAJOR;
    context->gd->first_minor = 0;
    context->gd->fops = &eblock_fops;
    context->gd->private_data = context;
    strcpy(context->gd->disk_name, name);

    //FIXME:改变大小
    set_capacity(context->gd, NSECTORS);
    context->gd->queue = context->queue;
    add_disk(context->gd);
    return 0;
}

//清理结构中的临时内存
static void  __destoryDev(struct EBlockContext *context)
{
    del_gendisk(context->gd);
    put_disk(context->gd);
    blk_cleanup_queue(context->queue);
}

//卸载一个块设备
void destoryEblockDev(const char * name)
{
    struct EBlockContext *context = findEBlockContext(name);
    __destoryDev(context);
    removeEBlockContext(name);
}

//清除所有dev设备
void clearEblockDevs(void)
{
    completeAllQueueRequests(-EIO);
    completeAllPendingEBRequests(-EIO);
    //清空context时遍历调用删除
    clearEBlockContexts(__destoryDev);
}

EXPORT_SYMBOL(createEblockDev);
EXPORT_SYMBOL(destoryEblockDev);
EXPORT_SYMBOL(clearEblockDevs);