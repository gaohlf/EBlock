//
// Created by 20075 on 2024/6/12.
//

#include "EBLockGlobalCond.h"
#include "EBlockRequestQueue.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

static DEFINE_SPINLOCK(condSpinlock);
static DECLARE_WAIT_QUEUE_HEAD(wait_queue);
static int waiters = 0;
static int condition = 0;

/***等待被唤醒  超时返回 -1，未超时返回0 ****/
void waitEBlockCond(void)
{
    printk(KERN_INFO "EBlockGlobalCond::waitEBlockCond IN waiters = %d\n", waiters);
    spin_lock(&condSpinlock);
    condition = 0;
    ++waiters;
    spin_unlock(&condSpinlock);

    wait_event_interruptible_timeout(wait_queue, condition, EBLOCK_WAIT_TO_FETCH_IO_TIMEOUT_SECS * HZ);
    spin_lock(&condSpinlock);
    --waiters;
    spin_unlock(&condSpinlock);
    printk(KERN_INFO "EBlockGlobalCond::waitEBlockCond OUT waiters = %d\n", waiters);
}

void wakeupEBlockCond(void)
{
    printk(KERN_INFO "EBlockGlobalCond::wakeupEBlockCond IN waiters = %d\n", waiters);
    spin_lock(&condSpinlock);
    if(waiters > 0)
    {
        condition = 1;
        wake_up_interruptible(&wait_queue);
    }
    spin_unlock(&condSpinlock);
    printk(KERN_INFO "EBlockGlobalCond::wakeupEBlockCond OUT waiters = %d\n", waiters);
}

EXPORT_SYMBOL(waitEBlockCond);
EXPORT_SYMBOL(wakeupEBlockCond);