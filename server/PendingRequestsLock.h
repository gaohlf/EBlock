//
// Created by 20075 on 2024/6/28.
//

#ifndef EBLOCK_PENDINGREQUESTS_H
#define EBLOCK_PENDINGREQUESTS_H
#include <list>
#include <map>
#include <mutex>
#include <condition_variable>
#include "../include/EBlockRequests.h"

/** 主要作用是读写相同区域时上锁
 * 主要提供同步锁和异步锁异
 *  异步锁暂不实现
 */
class PendingRequestsLock {
    std::map<unsigned long, struct EBRequest *> outstandingRequests;
    std::map<unsigned long, struct EBRequest *> doingRquests;
    std::mutex m;
    std::condition_variable cond;
private:
    /**
     * 如果新完成的request 与outstandingRequest 冲突 返回true
     * req为写时，doingrequest存在范围交叉的EBRequest 就为冲突
     * req为读时，doingrequest 与之范围交叉的EBRequest 有为写request 也是冲突
     */
    bool conflictWithoutstandingRequest(struct EBRequest *req);

    /**
     * 如果新写入的request 与doingRequests 冲突 返回true
     * req为写时，doingrequest存在范围交叉的EBRequest 就为冲突
     * req为读时，doingrequest 与之范围交叉的EBRequest 有为写request 也是冲突
     */
    bool conflictWithDoingRequest(struct EBRequest *req);


    /**
    * 除去自身以外是否还与 Outstanding其他IO冲突
    */
    bool conflictWithoutstandingRequestBesideReq( struct EBRequest *req);
public:
    PendingRequestsLock(){}
    ~PendingRequestsLock(){}

    //同步上锁 根据EBRequest 对IO范围进行上锁
    void getLockSync(struct EBRequest *req);

    //同步上锁 根据EBRequest 对IO范围进行上锁
    void releaseLockSync(struct EBRequest *req);
};


#endif //EBLOCK_PENDINGREQUESTS_H
