//
// Created by 20075 on 2024/6/11.
//


#include "IODispatch.h"
#include "EBlockIoctl.h"
#include "EblockErrors.h"
#include "Log4Eblock.h"
#include <unistd.h>

//开启调度服务并阻塞等待
void IODispatch::startWork() {
        ELOG("enter %p", this);
        // Create and start the thread, passing 'this' to the thread function
        std::thread workerThread(&IODispatch::threadSelectFromEblock, this);
        ELOG("before join %p", this);
        // Wait for the thread to complete
        workerThread.join();
}

void IODispatch::threadSelectFromEblock() {
        // This function can access the member variables and methods of the class
    ELOG("Thread is running. Object address: %p",this);
    while (true)
    {
        struct EBRequests requests;
        requests.requestNum = 0;
        int ret = 0;
//        ELOG("Before fetch requests.requestNum = %lu ret = %d", requests.requestNum, ret);
        ret = fetchRequests(&requests);
//        ELOG("After fetch requests.requestNum = %lu ret = %d", requests.requestNum, ret);
        if(ret != EBLOCK_ERROR_TYPE_OK || requests.requestNum == 0)
        {
            ELOG("fetchRequests timeout error %d\n", ret);
            continue;
        }

//        ELOG("requests.requestNum = %llu ret = %d", requests.requestNum, ret);
        for(u64 i = 0; i < requests.requestNum; ++i)
        {
            struct EBRequestDoneCtx ctx;
            ctx.kernelID = requests.requests[i].kernelID;

            //FIXME: 变成S3 转发到其他线程
            ctx.err = EBLOCK_ERROR_TYPE_OK;
            ret = requestDone(&ctx);
//            ELOG("requestDone success %d\n", ret);
            if(ret != EBLOCK_ERROR_TYPE_OK)
            {
                ELOG("requestDone failed %d\n", ret);
            }
        }
    }
}
