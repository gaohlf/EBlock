//
// Created by 20075 on 2024/6/11.
//


#include "IODispatch.h"
#include "EBlockIoctl.h"
#include "EblockErrors.h"
#include "Log4Eblock.h"
#include "IOTask.h"
#include "common/EBlockThreads.hpp"
#include <unistd.h>
#include <cassert>
#include "common/EBlockRequestsCommon.h"

extern EBlockThreads taskThreads;

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
            //必须是扇区的整数倍
            assert(requests.requests[i].off % 512 ==0);
            //生成task
            IOTask *task = new IOTask(&requests.requests[i]);
            ELOG_INFO("task [%p] request [%s]", task, toString(requests.requests[i]).c_str());
            assert(task != nullptr);
            taskThreads.enqueue(std::bind(&IOTask::process, task));
        }
    }
}
