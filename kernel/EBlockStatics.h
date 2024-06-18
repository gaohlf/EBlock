//
// Created by 20075 on 2024/6/13.
//

#ifndef EBLOCK_EBLOCKSTATICS_H
#define EBLOCK_EBLOCKSTATICS_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

struct EBStaics{
    u64 ioStartNum;
    u64 ioCompleteNum;
    u64 ioToFetchNum;
    u64 ioStartPendingNum;
    u64 ioEndPendingNum;
    u64 ioFetchedNum;
    u64 ioReadByUserNum;
    u64 ioWrittenByUserNum;
    u64 ioDoneByUserNum;
};

//一个IO开始了 +
extern void ioStart(void);

//一个IO结束了 +
extern void ioComplete(void);

//一个IO 进入了fetch队列 +
extern void ioToFetch(void);

//一个IO 进入了pendingmap
extern void ioStartPending(void);

//一个IO 一移出了pendingmap
extern void ioEndPending(void);

//一个IO 被用户态取走了 +
extern void ioFetched(void);

//一个IO被用户读走了 -
extern void ioReadByUser(void);

//一个IO被用户写入了 -
extern void ioWrittenByUser(void);

//一个IO被用户调用了完成
extern void ioDoneByUser(void);

//打印所有挂起状态
extern void dumpAllStatics(void);

#endif //EBLOCK_EBLOCKSTATICS_H
