//
// Created by 20075 on 2024/6/28.
//

#include "common/EBlockTypes.h"
#include "PendingRequestsLock.h"

//是否有读写区域冲突，读读不冲突，
static bool conflictWithRequest( struct EBRequest *reqA,  struct EBRequest *reqB)
{
    //两者都是读 不冲突
    if(!reqA->isWrite && !reqB->isWrite)
    {   return false;
    }
    //范围不交叉不冲突
    if(reqA->off + reqA->length <= reqB->off || //A的线段在B的左侧
       reqB->off + reqB->length <= reqA->off) //A的线段在B的右侧
    {
        return false;
    }
    return true;
}

/**
 * 与除了req之外的 所有req是否有冲突
 */
static bool conflictWithRequestsBesideReq(std::map<unsigned long, struct EBRequest *> &reqMap, struct EBRequest *req)
{
    for(auto it = reqMap.begin(); it != reqMap.end(); ++it)
    {
        struct EBRequest *cmpReq = it->second;
        if(cmpReq->kernelID == req->kernelID)
        {//相同ID不做比较
            continue;
        }
        if(conflictWithRequest(req,cmpReq))
        {
            return true;
        }
    }
    return false;
}

/**
 * 如果新完成的request 与Requests 冲突 返回true
 * req为写时，doingrequest存在范围交叉的EBRequest 就为冲突
 * req为读时，doingrequest 与之范围交叉的EBRequest 有为写request 也是冲突
 */
static bool conflictWithRequests(std::map<unsigned long, struct EBRequest *> &reqMap, struct EBRequest *req)
{
    for(auto it = reqMap.begin(); it != reqMap.end(); ++it)
    {
        struct EBRequest *cmpReq = it->second;
        if(conflictWithRequest(req,cmpReq))
        {
            return true;
        }
    }
    return false;
}

/**
 * 如果新完成的request 与outstandingRequest 冲突 返回true
 * req为写时，doingrequest存在范围交叉的EBRequest 就为冲突
 * req为读时，doingrequest 与之范围交叉的EBRequest 有为写request 也是冲突
 */
bool PendingRequestsLock::conflictWithoutstandingRequest( struct EBRequest *req)
{
    return conflictWithRequests(outstandingRequests, req);
}

/**
 * 除去自身以外是否还与 Outstanding其他IO冲突
 */
bool PendingRequestsLock::conflictWithoutstandingRequestBesideReq( struct EBRequest *req)
{
    return conflictWithRequestsBesideReq(outstandingRequests, req);
}


/**
 * 如果新写入的request 与doing request 冲突 返回true
 * req为写时，doingrequest存在范围交叉的EBRequest 就为冲突
 * req为读时，doingrequest 与之范围交叉的EBRequest 有为写request 也是冲突
 */
bool PendingRequestsLock::conflictWithDoingRequest(struct EBRequest *req)
{
    return conflictWithRequests(doingRquests, req);;
}

//同步上锁 根据EBRequest 对IO范围进行上锁
void PendingRequestsLock::getLockSync(struct EBRequest *req)
{
    unsigned long id = req->kernelID;
    std::unique_lock<std::mutex> lck(m);
    if(!this->conflictWithDoingRequest(req) && !this->conflictWithoutstandingRequest(req))
    {
        doingRquests.insert(std::make_pair(id, req));
        return;
    }
    outstandingRequests.insert(std::make_pair(id, req));

    while(true){//一直冲突一直等
        cond.wait(lck);
        if(!this->conflictWithDoingRequest(req))
        {
            doingRquests.insert(std::make_pair(id, req));
            outstandingRequests.erase(id);
            //NOTE:这里面不保证互斥顺序
            return;
        }
    }
}

//同步上锁 根据EBRequest 对IO范围进行上锁
void PendingRequestsLock::releaseLockSync(struct EBRequest *req)
{
    std::unique_lock<std::mutex> lck(m);
    doingRquests.erase((unsigned long)req->kernelID);
    if(conflictWithoutstandingRequest(req))//有冲突说明有人需要唤醒重新检查是否 仍有冲突
    {
        cond.notify_all();
    }
}