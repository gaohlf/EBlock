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

static void dumpBioFlags(struct bio *bio)
{
    printk(KERN_INFO "--------------------Start ECreateBlkDev::dumpBioFlags bio = %p----------------------------\n",bio);
    printk(KERN_INFO "++bio->bi_rw = 0x%016lx (isWrite:%d)---------------\n", bio->bi_rw,
           bio_data_dir(bio) == WRITE);
    printk(KERN_INFO "++bio->bi_flags = 0x%016lx-------------------------\n", bio->bi_flags);
    printk(KERN_INFO "++bio->bi_sector = 0x%016lx (%lu)-------------\n", bio->bi_sector,
           bio->bi_sector * SECTOR_SIZE);
    printk(KERN_INFO "++bio->bi_size = %u----------------------------\n", bio->bi_size);
    printk(KERN_INFO "-----------------------END ECreateBlkDev::dumpBioFlags bio = %p----------------------------\n",bio);

}

//返回true则跳过该bio
static bool ignoreBios( struct bio *bio)
{
    //控制命令BIO 直接返回成功
    if (bio->bi_rw & REQ_DISCARD) {
        printk(KERN_INFO "Skipping control command for bio: %p\n",bio);
        dumpBioFlags(bio);
        bio_endio(bio, 0); // 完成 BIO 请求
        return true;
    }

    return false;
}

/**
 * 数据IO的关键函数
 * ************************************************************************/
//代替IO处理
static void eblockMakeRequest(struct request_queue *q, struct bio *bio)
{
    int ret = 0;
    struct EBlockContext * context = NULL;
    printk(KERN_INFO "ECreateBlkDev::eblockMakeRequest IN q = %p, bio = %p\n", q, bio);
    ioStart();//取到一个IO

    if(ignoreBios(bio))
    {//FIXME:区分 忽略IO
        ioComplete();
        return;
    }
    //NOTE:打印bio的详细信息
    dumpBioFlags(bio);

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