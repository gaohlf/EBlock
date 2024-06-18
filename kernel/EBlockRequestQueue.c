#include "EBlockRequestQueue.h"
#include "EBlockRequestManager.h"
#include "EBlockStatics.h"

static LIST_HEAD(requestQueue);
static DEFINE_SPINLOCK(queueSpinlock);

/** 判断等待被取走的队列是否为空 ***/
bool requestQueueEmpty(void)
{
    bool ret = false;
    spin_lock(&queueSpinlock);
    if(list_empty( &requestQueue))
    {
        ret = true;
    }
    spin_unlock(&queueSpinlock);

    return ret;
}

void enRequestQueue(struct EBRequestInKernel *requestInKernel)
{
    ioToFetch();
//    printk(KERN_INFO "EBlockRequesQueue::enRequestQueue %p \n", requestInKernel);
    spin_lock(&queueSpinlock);
    INIT_LIST_HEAD(&requestInKernel->list);
    list_add_tail(&requestInKernel->list, &requestQueue);
    spin_unlock(&queueSpinlock);
}

//将request列表取出到head中，一次至多取出MAX_SWAP_REQUESTS_ONCE个请求
int fetchRequestQueueOnce(struct list_head *out_list)
{
    int count = 0;
    struct EBRequestInKernel *requestInKernel, *tmp;
    spin_lock(&queueSpinlock);
    //遍历队列从前面找出至多 MAX_SWAP_REQUESTS_ONCE个请求出队列
    list_for_each_entry_safe(requestInKernel, tmp, &requestQueue, list) {
        if (count >= MAX_SWAP_REQUESTS_ONCE) {
            break;
        }
        list_del(&requestInKernel->list);
        list_add_tail(&requestInKernel->list, out_list);
        count++;
        ioFetched();
    }

    spin_unlock(&queueSpinlock);
//    printk(KERN_INFO "EBlockRequesQueue::fetchRequestQueueOnce Dequeued %d elements\n", count);
    return count;
}

//根据listhead获取对应EBRequestInKernel
struct EBRequestInKernel *requestByListHead(struct list_head * pos)
{
    struct EBRequestInKernel *requestInKernel = list_entry(pos, struct EBRequestInKernel, list);
    return requestInKernel;
}

//取空所有队列的请求，并使其报错
void completeAllQueueRequests(int err)
{
    LIST_HEAD(out_list);
    struct EBRequestInKernel *requestInKernel, *tmp;
    spin_lock(&queueSpinlock);
    //遍历队列从前面找出至多 M:1AX_SWAP_REQUESTS_ONCE个请求出队列
    list_for_each_entry_safe(requestInKernel, tmp, &requestQueue, list) {
        list_del(&requestInKernel->list);
        list_add_tail(&requestInKernel->list, &out_list);
    }
    spin_unlock(&queueSpinlock);

    //从队列中摘出来后 再行销毁内存
    list_for_each_entry_safe(requestInKernel, tmp, &out_list, list){
        printk(KERN_INFO "EBlockRequesQueue::completeAllQueueRequests Dequeued %llu elements\n", requestInKernel->req.kernelID);
        completeEBRequest(requestInKernel, err);
    }
}

//打印队列信息
void dumpAllQueueRequests(void)
{
    LIST_HEAD(out_list);
    struct EBRequestInKernel *requestInKernel, *tmp;
    printk(KERN_ERR "------------------------- EBlockRequesQueue::dumpAllQueueRequests ---------------------------\n");
    spin_lock(&queueSpinlock);
    //遍历队列从前面找出至多 M:1AX_SWAP_REQUESTS_ONCE个请求出队列
    list_for_each_entry_safe(requestInKernel, tmp, &requestQueue, list) {
        dumpEBRequestK(requestInKernel);
    }
    spin_unlock(&queueSpinlock);
}

EXPORT_SYMBOL(requestQueueEmpty);
EXPORT_SYMBOL(enRequestQueue);
EXPORT_SYMBOL(fetchRequestQueueOnce);
EXPORT_SYMBOL(requestByListHead);
EXPORT_SYMBOL(completeAllQueueRequests);
EXPORT_SYMBOL(dumpAllQueueRequests);