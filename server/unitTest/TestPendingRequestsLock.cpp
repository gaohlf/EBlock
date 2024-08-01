//
// Created by 20075 on 2024/7/3.
//
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <cassert>
#include "../common/EBlockTypes.h"
#include "TestPendingRequestsLock.h"
#include "PendingIDs.h"
#include "SafeCounter.h"
#define GREEN_START "\033[32m"
#define YELLOW_START "\033[33m"
#define COLOE_RESET "\033[0m"

#ifdef __WIN64__
#include <windows.h>

void EnableVirtualTerminalProcessing() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Invalid handle");
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
        throw std::runtime_error("GetConsoleMode failed");
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) {
        throw std::runtime_error("SetConsoleMode failed");
    }
}

#endif


PendingRequestsLock lck;

int getRandomNumber(int min, int max) {
    // 生成随机数引擎
    std::random_device rd; // 用于获取随机种子
    std::mt19937 gen(rd()); // 使用Mersenne Twister引擎

    // 定义分布范围
    std::uniform_int_distribution<> dis(min, max);

    // 生成随机数
    return dis(gen);
}

typedef void makeEBRequestFunc(int id,struct EBRequest &ebRequest);

//读读无冲突情景上锁
void threadcase(int id, makeEBRequestFunc func, PendingIDs *ids) {

    struct EBRequest ebRequest;
    func(id, ebRequest);
    lck.getLockSync(&ebRequest);
//    std::cout << "-----Thread threadcase " << id << " lock got" << std::endl;
    ids->recordIDNow(id);

    std::this_thread::sleep_for(std::chrono::seconds(getRandomNumber(1,1)));

    ids->removeIDNow(id);
//    std::cout << "----Thread " << id << " ready to unlock" << std::endl;
    lck.releaseLockSync(&ebRequest);
}

void startThreads(const int numThreads, makeEBRequestFunc func,PendingIDs &ids)
{
    std::vector<std::thread> threads;

    // 启动多个线程 模拟随机范围上锁解锁
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(threadcase, i, func, &ids);
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}


void case0MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 500;
    ebRequest.off = id * ebRequest.length;
    ebRequest.isWrite = false;
}

//读读无冲突情景上锁
void case0ReadReadNoConflict()
{
    PendingIDs ids;
    std::cout << "0) case0ReadReadNoConflict "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(2, case0MakeEBRequest, ids);
    assert(ids.doTheyExecInSameTime(0, 1));

    std::cout << "0) case0ReadReadNoConflict "<< GREEN_START << "OK" << COLOE_RESET << std::endl;
}

void case1MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 500;
    ebRequest.off = id * ebRequest.length/2;
    ebRequest.isWrite = false;
}

//读读有冲突情景上锁
void case1ReadReadConflict()
{
    PendingIDs ids;
    std::cout << "1) case1ReadReadConflict "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(2, case1MakeEBRequest, ids);
    assert(ids.doTheyExecInSameTime(0, 1));

    std::cout << "1) case1ReadReadConflict " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}

//读写有无冲突情景上锁
void case2MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 500;
    ebRequest.off = id * ebRequest.length;
    if(id ==0 )
        ebRequest.isWrite = false;
    else
        ebRequest.isWrite = true;
}


//读写有无冲突情景上锁
void case2ReadWriteNoConflict()
{
    PendingIDs ids;
    std::cout << "2) case2ReadWriteNoConflict "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(2, case2MakeEBRequest, ids);
    assert(ids.doTheyExecInSameTime(0, 1));

    std::cout << "2) case2ReadWriteNoConflict " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}


//读写有冲突情景上锁
void case3MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 500;
    ebRequest.off = id * ebRequest.length/2;
    if(id ==0 )
        ebRequest.isWrite = false;
    else
        ebRequest.isWrite = true;
}

//读写有冲突情景上锁
void case3ReadWriteConflict()
{
    PendingIDs ids;
    std::cout << "3) case3ReadWriteConflict "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(2, case3MakeEBRequest, ids);
    assert(!ids.doTheyExecInSameTime(0, 1));

    std::cout << "3) case3ReadWriteConflict " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}

//写写有冲突情景上锁
void case4MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 500;
    ebRequest.off = id * ebRequest.length/2;
    ebRequest.isWrite = true;
}

//写写有冲突情景上锁
void case4WriteWriteConflict()
{
    PendingIDs ids;
    std::cout << "4) case4WriteWriteConflict "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(2, case4MakeEBRequest, ids);
    assert(!ids.doTheyExecInSameTime(0, 1));

    std::cout << "4) case4WriteWriteConflict " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}


//写写不冲突情景上锁
void case5MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 500;
    ebRequest.off = id * ebRequest.length;
    ebRequest.isWrite = true;
}

//写写不冲突情景上锁
void case5WriteWriteNoConflict()
{
    PendingIDs ids;
    std::cout << "5) case5WriteWriteNoConflict "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(2, case5MakeEBRequest, ids);
    assert(ids.doTheyExecInSameTime(0, 1));

    std::cout << "5) case5WriteWriteNoConflict " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}

//读/读/写 互为冲突
void case6MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 600;
    ebRequest.off = id * ebRequest.length/3;
    if(id == 2)
    {
        ebRequest.isWrite = true;
    }
    else
    {
        ebRequest.isWrite = false;
    }
}

//读/读/写 互为冲突
void case6ReadReadWriteALLConflict()
{
    PendingIDs ids;
    std::cout << "6) case5WriteWriteNoConflict "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(3, case6MakeEBRequest, ids);

    assert(ids.doTheyExecInSameTime(0, 1));
    assert(!ids.doTheyExecInSameTime(0, 2));
    assert(!ids.doTheyExecInSameTime(1, 2));

    std::cout << "6) case6ReadReadWriteALLConflict " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}

//读/读/写 仅与写冲突 读读之间不冲突
void case7MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 300;
    if(id == 2)
    {
        ebRequest.off = 0;
        ebRequest.length *= 2;
    }
    else
    {
        ebRequest.off = id * ebRequest.length;
    }

    if(id == 2)
    {
        ebRequest.isWrite = true;
    }
    else
    {
        ebRequest.isWrite = false;
    }
}

//读/读/写 仅与写冲突 读读之间不冲突
void case7ReadReadWriteConflictOnlyWithWrite()
{
    PendingIDs ids;
    std::cout << "7) case7ReadReadWriteConflictOnlyWithWrite "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(3, case7MakeEBRequest, ids);

    assert(ids.doTheyExecInSameTime(0, 1));
    assert(!ids.doTheyExecInSameTime(0, 2));
    assert(!ids.doTheyExecInSameTime(1, 2));

    std::cout << "7) case7ReadReadWriteConflictOnlyWithWrite " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}

//读/读/写 读与读冲突 但不与写冲突
void case8MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 300;
    if(id == 2)
    {
        ebRequest.off =  id * ebRequest.length;
    }
    else
    {
        ebRequest.off = id * ebRequest.length/2;
    }

    if(id == 2)
    {
        ebRequest.isWrite = true;
    }
    else
    {
        ebRequest.isWrite = false;
    }
}


//读/读/写 读与读冲突 但不与写冲突
void case8ReadReadWriteConflictWithRead()
{
    PendingIDs ids;
    std::cout << "8) case8ReadReadWriteConflictWithRead "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(3, case8MakeEBRequest, ids);

    assert(ids.doTheyExecInSameTime(0, 1));
    assert(ids.doTheyExecInSameTime(0, 2));
    assert(ids.doTheyExecInSameTime(1, 2));

    std::cout << "8) case8ReadReadWriteConflictWithRead " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}

//读/写/写  其中一个写和读冲突 读先行 另一个写不冲突
void case9MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 300;
    if(id == 2)
    {
        ebRequest.off =  id * ebRequest.length;
    }
    else
    {
        ebRequest.off = id * ebRequest.length/2;
    }

    if(id == 0)
    {
        ebRequest.isWrite = false;
    }
    else
    {
        ebRequest.isWrite = true;
    }
}

//读/写/写  其中一个写和读冲突 读先行 另一个写不冲突
void case9ReadWriteWriteConflictWithRead()
{
    PendingIDs ids;
    std::cout << "9) case9ReadWriteWriteConflictWithRead "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(3, case9MakeEBRequest, ids);

    assert(!ids.doTheyExecInSameTime(0, 1));
    assert(ids.doTheyExecInSameTime(0, 2));

    std::cout << "9) case9ReadWriteWriteConflictWithRead " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}


//读/写/写 读与写不冲突 两个写冲突
void case10MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 300;
    if(id == 0)
    {
        ebRequest.off =  3 * ebRequest.length;
    }
    else
    {
        ebRequest.off = (id-1) * ebRequest.length/2;
    }

    if(id == 0)
    {
        ebRequest.isWrite = false;
    }
    else
    {
        ebRequest.isWrite = true;
    }
}


//读/写/写 读与写不冲突 两个写冲突
void case10ReadWriteWriteConflictWithWrite()
{
    PendingIDs ids;
    std::cout << "10) case10ReadWriteWriteConflictWithWrite "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(3, case10MakeEBRequest, ids);

    assert(ids.doTheyExecInSameTime(0, 1));
    assert(!ids.doTheyExecInSameTime(1, 2));

    std::cout << "10) case10ReadWriteWriteConflictWithWrite " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}

//读/读/读 三者两两冲突
void case11MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 300;
    ebRequest.off = id * ebRequest.length/3;
    ebRequest.isWrite = false;
}

//读/读/读 三者两两冲突
void case11ReadReadReadConflictWithEachOther()
{
    PendingIDs ids;
    std::cout << "11) case11ReadReadReadConflictWithEachOther "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(3, case11MakeEBRequest, ids);

    assert(ids.doTheyExecInSameTime(0, 1));
    assert(ids.doTheyExecInSameTime(1, 2));
    assert(ids.doTheyExecInSameTime(0, 2));

    std::cout << "11) case11ReadReadReadConflictWithEachOther " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}

//写/写/写 三者两两冲突
void case12MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 300;
    ebRequest.off = id * ebRequest.length/3;
    ebRequest.isWrite = true;
}

//写/写/写 三者两两冲突
void case12WriteWriteWriteConflictWithEachOther()
{
    PendingIDs ids;
    std::cout << "12) case12WriteWriteWriteConflictWithEachOther "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(3, case12MakeEBRequest, ids);

    assert(!ids.doTheyExecInSameTime(0, 1));
    assert(!ids.doTheyExecInSameTime(1, 2));
    assert(!ids.doTheyExecInSameTime(0, 2));

    std::cout << "12) case12WriteWriteWriteConflictWithEachOther " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}

//写/读/写 两个写不冲突，但都与读冲突
void case13MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 300;
    ebRequest.off = id * ebRequest.length/2;


    if(id == 1)
    {
        ebRequest.isWrite = false;
    }
    else
    {
        ebRequest.isWrite = true;
    }
}

//写/读/写 两个写不冲突，但都与读冲突
void case13WriteReadWriteConflictWithEachOther()
{
    PendingIDs ids;
    std::cout << "13) case13WriteReadWriteConflictWithEachOther "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(3, case13MakeEBRequest, ids);

    assert(!ids.doTheyExecInSameTime(0, 1));
    assert(!ids.doTheyExecInSameTime(1, 2));
    assert(!ids.doTheyExecInSameTime(0, 2));

    std::cout << "13) case13WriteReadWriteConflictWithEachOther " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}

//写/写/读 读与写冲突 两个写不冲突 写先行
void case14MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 300;
    ebRequest.off = id * ebRequest.length;
    if(id == 2)
    {
        ebRequest.off = 0;
        ebRequest.length *= 2;
    }

    if(id == 2)
    {
        ebRequest.isWrite = false;
    }
    else
    {
        ebRequest.isWrite = true;
    }
}

//写/写/读 读与写冲突 两个写不冲突 写先行
void case14WriteWriteReadConflictWithRead()
{
    PendingIDs ids;
    std::cout << "14) case14WriteWriteReadConflictWithRead "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(3, case14MakeEBRequest, ids);

    assert(ids.doTheyExecInSameTime(0, 1));
    assert(!ids.doTheyExecInSameTime(1, 2));
    assert(!ids.doTheyExecInSameTime(0, 2));

    std::cout << "14) case14WriteWriteReadConflictWithRead " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}


//读/写/读 两个读不冲突，但都与写冲突
void case15MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 300;
    ebRequest.off = id * ebRequest.length/2;

    if(id == 1)
    {
        ebRequest.isWrite = true;
    }
    else
    {
        ebRequest.isWrite = false;
    }
}

//读/写/读 两个读不冲突，但都与写冲突
void case15ReadWriteReadConflictWithEachOther()
{
    PendingIDs ids;
    std::cout << "15) case15ReadWriteReadConflictWithEachOther "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(3, case15MakeEBRequest, ids);

    assert(!ids.doTheyExecInSameTime(0, 1));
    assert(!ids.doTheyExecInSameTime(1, 2));
    assert(!ids.doTheyExecInSameTime(0, 2));

    std::cout << "15) case15ReadWriteReadConflictWithEachOther " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}


//写/写/写 第一个和第三个不冲突但都与第二个冲突
void case16MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 300;
    ebRequest.off = id * ebRequest.length/2;
    ebRequest.isWrite = true;
}

//写/写/写 第一个和第三个不冲突但都与第二个冲突
void case16WriteWriteWriteConflictOneByOne()
{
    PendingIDs ids;
    std::cout << "16) case16ReadWriteReadConflictWithEachOther "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(3, case16MakeEBRequest, ids);

    assert(!ids.doTheyExecInSameTime(0, 1));
    assert(!ids.doTheyExecInSameTime(1, 2));
    assert(!ids.doTheyExecInSameTime(0, 2));

    std::cout << "16) case16ReadWriteReadConflictWithEachOther " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}

//读/读/读 第一个和第三个不冲突但都与第二个冲突
void case17MakeEBRequest(int id,struct EBRequest &ebRequest)
{
    ebRequest.kernelID = id;
    ebRequest.length = 300;
    ebRequest.off = id * ebRequest.length/2;
    ebRequest.isWrite = false;
}

//读/读/读 第一个和第三个不冲突但都与第二个冲突
void case17ReadReadReadConflictOneByOne()
{
    PendingIDs ids;
    std::cout << "17) case17ReadReadReadConflictOneByOne "<< YELLOW_START <<"start" << COLOE_RESET << std::endl;

    startThreads(3, case17MakeEBRequest, ids);

    assert(ids.doTheyExecInSameTime(0, 1));
    assert(ids.doTheyExecInSameTime(1, 2));
    assert(ids.doTheyExecInSameTime(0, 2));

    std::cout << "17) case17ReadReadReadConflictOneByOne " << GREEN_START << "OK" << COLOE_RESET  << std::endl;
}

int main(int argc, const char * argv[])
{
#ifdef __WIN64__
    EnableVirtualTerminalProcessing();
#endif
    case0ReadReadNoConflict();

    case1ReadReadConflict();

    case2ReadWriteNoConflict();

    case3ReadWriteConflict();

    case4WriteWriteConflict();

    case5WriteWriteNoConflict();

    case6ReadReadWriteALLConflict();

    case7ReadReadWriteConflictOnlyWithWrite();

    case8ReadReadWriteConflictWithRead();

    case9ReadWriteWriteConflictWithRead();

    case10ReadWriteWriteConflictWithWrite();

    case11ReadReadReadConflictWithEachOther();

    case12WriteWriteWriteConflictWithEachOther();

    case13WriteReadWriteConflictWithEachOther();

    case14WriteWriteReadConflictWithRead();

    case15ReadWriteReadConflictWithEachOther();

    case16WriteWriteWriteConflictOneByOne();

    case17ReadReadReadConflictOneByOne();

    std::cout << "ALL cases " << GREEN_START << "Complete" << COLOE_RESET  << std::endl;
    return 0;
}