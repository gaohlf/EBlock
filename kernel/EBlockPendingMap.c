#include "EBlockPendingMap.h"
#include "EBlockRequestManager.h"
#include "EBlockStatics.h"
#define HASH_BITS_NUM 10
static DEFINE_HASHTABLE(pendingMap, HASH_BITS_NUM);
static DEFINE_SPINLOCK(pendingSpinlock);

//从等待队列中取出后，进入pendingmap中的函数
void startPendingRequest(struct EBRequestInKernel * requestInKernel)
{
    ioStartPending();
    spin_lock(&pendingSpinlock);
    hash_add(pendingMap, &requestInKernel->hashnode, requestInKernel->req.kernelID);
    spin_unlock(&pendingSpinlock);
}

//根据kernelID查询pending的request
struct EBRequestInKernel * findPendingRequest(u64 kernelID)
{
    struct EBRequestInKernel * requestInKernel;
    spin_lock(&pendingSpinlock);
    hash_for_each_possible(pendingMap, requestInKernel, hashnode, kernelID) {
        if (requestInKernel->req.kernelID == kernelID) {
            spin_unlock(&pendingSpinlock);
//            printk(KERN_INFO "EBlockPendingMap::findPendingRequest : key = %llu\n", key);
            return requestInKernel;
        }
    }
    spin_unlock(&pendingSpinlock);
    printk(KERN_ERR "EBlockPendingMap::findPendingRequest Not found: kernelID = %llu\n", kernelID);
    return NULL;
}

//从pending队列中出并完成的函数
void endPendingRequest(u64 kernelID, int err)
{
    struct EBRequestInKernel * requestInKernel;
    spin_lock(&pendingSpinlock);
    hash_for_each_possible(pendingMap, requestInKernel, hashnode, kernelID) {
        if (requestInKernel->req.kernelID == kernelID) {
            hash_del(&requestInKernel->hashnode);
            ioEndPending();
            spin_unlock(&pendingSpinlock);
            //            printk(KERN_INFO "EBlockPendingMap::endPendingRequest : key = %llu\n", key);
            completeEBRequest(requestInKernel, err);
            return;
        }
    }
    spin_unlock(&pendingSpinlock);
    printk(KERN_ERR "EBlockPendingMap::endPendingRequest Not found: kernelID = %llu\n", kernelID);
    return;
}

//结束所有挂起EBrequest的生命周期 结束对于pending的request里的生命周期
void completeAllPendingEBRequests(int err)
{
    LIST_HEAD(out_list);
    int bkt;
    struct EBRequestInKernel * requestInKernel, *tmpreq;
    struct hlist_node *tmp;
    spin_lock(&pendingSpinlock);
    hash_for_each_safe(pendingMap, bkt, tmp, requestInKernel, hashnode) {
        hash_del(&requestInKernel->hashnode);
        list_add_tail(&requestInKernel->list, &out_list);
        ioEndPending();
    }
    spin_unlock(&pendingSpinlock);

    list_for_each_entry_safe(requestInKernel, tmpreq, &out_list, list){
        printk(KERN_INFO "EBlockPendingMap::completeAllPendingEBRequests Dequeued %llu elements\n", requestInKernel->req.kernelID);
        completeEBRequest(requestInKernel, err);
    }
}

//打印所有挂起EBrequest的
void dumpAllPendingEBRequests(void)
{
    LIST_HEAD(out_list);
    int bkt;
    struct EBRequestInKernel * requestInKernel;
    struct hlist_node *tmp;
    printk(KERN_ERR "------------------------- EBlockPendingMap::dumpAllPendingEBRequests ---------------------------\n");
    spin_lock(&pendingSpinlock);
    hash_for_each_safe(pendingMap, bkt, tmp, requestInKernel, hashnode) {
        dumpEBRequestK(requestInKernel);
    }
    spin_unlock(&pendingSpinlock);
}

EXPORT_SYMBOL(startPendingRequest);
EXPORT_SYMBOL(findPendingRequest);
EXPORT_SYMBOL(endPendingRequest);
EXPORT_SYMBOL(completeAllPendingEBRequests);
EXPORT_SYMBOL(dumpAllPendingEBRequests);