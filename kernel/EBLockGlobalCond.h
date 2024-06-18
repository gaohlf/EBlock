//
// Created by 20075 on 2024/6/12.
//

#ifndef EBLOCK_EBLOCKGLOBALCOND_H
#define EBLOCK_EBLOCKGLOBALCOND_H

#define EBLOCK_WAIT_TO_FETCH_IO_TIMEOUT_SECS 5

/***等待被唤醒  超时返回 -1，未超时返回0 ****/
extern void waitEBlockCond(void);

/****唤醒等待*********************************/
extern void wakeupEBlockCond(void);

#endif //EBLOCK_EBLOCKGLOBALCOND_H
