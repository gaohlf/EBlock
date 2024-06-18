//
// Created by 20075 on 2024/6/13.
//

#include "EBlockStatics.h"

static DEFINE_SPINLOCK(staticsSpinlock);

static struct EBStaics ebStaics =
{
    .ioStartNum = 0L,
    .ioCompleteNum = 0L,
    .ioToFetchNum = 0L,
    .ioStartPendingNum = 0L,
    .ioEndPendingNum = 0L,
    .ioFetchedNum = 0L,
    .ioReadByUserNum = 0L,
    .ioWrittenByUserNum = 0L,
    .ioDoneByUserNum = 0L
};

//一个IO开始了
void ioStart(void)
{
    spin_lock(&staticsSpinlock);
    ++ebStaics.ioStartNum;
    spin_unlock(&staticsSpinlock);
}

//一个IO结束了
void ioComplete(void)
{
    spin_lock(&staticsSpinlock);
    ++ebStaics.ioCompleteNum;
    spin_unlock(&staticsSpinlock);
}

//一个IO 进入了fetch队列
void ioToFetch(void)
{
    spin_lock(&staticsSpinlock);
    ++ebStaics.ioToFetchNum;
    spin_unlock(&staticsSpinlock);
}

//一个IO 进入了pendingmap
void ioStartPending(void)
{
    spin_lock(&staticsSpinlock);
    ++ebStaics.ioStartPendingNum;
    spin_unlock(&staticsSpinlock);
}

//一个IO 一移出了pendingmap
void ioEndPending(void)
{
    spin_lock(&staticsSpinlock);
    ++ebStaics.ioEndPendingNum;
    spin_unlock(&staticsSpinlock);
}

//一个IO 被用户态取走了
extern void ioFetched(void)
{
    spin_lock(&staticsSpinlock);
    ++ebStaics.ioFetchedNum;
    spin_unlock(&staticsSpinlock);
}

//一个IO被用户读走了
extern void ioReadByUser(void)
{
    spin_lock(&staticsSpinlock);
    ++ebStaics.ioReadByUserNum;
    spin_unlock(&staticsSpinlock);
}

//一个IO被用户写入了
extern void ioWrittenByUser(void)
{
    spin_lock(&staticsSpinlock);
    ++ebStaics.ioWrittenByUserNum;
    spin_unlock(&staticsSpinlock);
}

//一个IO被用户调用了完成
extern void ioDoneByUser(void)
{
    spin_lock(&staticsSpinlock);
    ++ebStaics.ioDoneByUserNum;
    spin_unlock(&staticsSpinlock);
}

//打印所有挂起状态
extern void dumpAllStatics(void)
{
    printk(KERN_ERR "------------------------- EBlockStatics::dumpAllStatics ---------------------------\n");
    printk(KERN_ERR "++ioStartNum:           %llu\n", ebStaics.ioStartNum);
    printk(KERN_ERR "++ioCompleteNum:        %llu\n", ebStaics.ioCompleteNum);
    printk(KERN_ERR "++ioStartPendingNum:    %llu\n", ebStaics.ioStartPendingNum);
    printk(KERN_ERR "++ioEndPendingNum:      %llu\n", ebStaics.ioEndPendingNum);
    printk(KERN_ERR "++ioFetchedNum:         %llu\n", ebStaics.ioFetchedNum);
    printk(KERN_ERR "++ioFetchedNum:         %llu\n", ebStaics.ioFetchedNum);
    printk(KERN_ERR "++ioReadByUserNum:      %llu\n", ebStaics.ioReadByUserNum);
    printk(KERN_ERR "++ioWrittenByUserNum:   %llu\n", ebStaics.ioWrittenByUserNum);
    printk(KERN_ERR "++ioDoneByUserNum:      %llu\n", ebStaics.ioDoneByUserNum);
}

EXPORT_SYMBOL(ioStart);
EXPORT_SYMBOL(ioComplete);
EXPORT_SYMBOL(ioToFetch);
EXPORT_SYMBOL(ioStartPending);
EXPORT_SYMBOL(ioEndPending);
EXPORT_SYMBOL(ioFetched);
EXPORT_SYMBOL(ioReadByUser);
EXPORT_SYMBOL(ioWrittenByUser);
EXPORT_SYMBOL(ioDoneByUser);
EXPORT_SYMBOL(dumpAllStatics);