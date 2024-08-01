#include <cassert>
#include "../Log4Eblock.h"
#include "../CacheDevice.h"
#include "../common/EColor.h"
#include "FakeClient.h"
#include <iostream>
#include <string>
#include <libgen.h>
#include <limits.h>
#include <cstring>

//
// Created by 20075 on 2024/7/25.
//
CacheDevice device;
static std::string programDir;

extern  std::string yellow(const std::string &org);
extern  std::string green(const std::string &org);
const unsigned long DataLength = 512*15L;

void testFillBuff(unsigned char *buff,const unsigned long length)
{
    static long gCnt = 0;
    for(auto i = 0; i < length; ++i)
    {
        buff[i] = gCnt % 26 + 'a';
        gCnt = (gCnt+1) % 26;
    }
}

//一次写写与读相同
void caseWriteReadOnce()
{
    ELOG_INFO("caseWriteReadOnce %s",yellow("started").c_str());
    struct EBRequest req;

    req.isWrite = 1; //读写标志
    req.length = DataLength;   //请求的字节数
    req.kernelID = 1;
    req.off = 0;

    char buff[DataLength] , cmpbuff[DataLength];
    testFillBuff(reinterpret_cast<unsigned char *>(buff), DataLength);
    assert( 0 == device.doRequestSync(&req, buff));
    req.isWrite = 0;
    req.kernelID ++;
    assert(0 == device.doRequestSync(&req, cmpbuff));
    assert(std::memcmp(buff, cmpbuff, DataLength) == 0);
    ELOG_INFO("caseWriteReadOnce %s",green("ok").c_str());
}

void caseReadWriteReadOnce()
{
    ELOG_INFO("caseReadWriteReadOnce %s",yellow("started").c_str());
    struct EBRequest req;

    req.isWrite = 0; //读写标志
    req.length = DataLength;   //请求的字节数
    req.kernelID = 1;

    req.off = 0;

    char buff[DataLength] , cmpbuff[DataLength];
    testFillBuff(reinterpret_cast<unsigned char *>(buff), DataLength);

    assert( 0 == device.doRequestSync(&req, buff));
    req.isWrite = 1;
    req.kernelID ++;
    assert(0 == device.doRequestSync(&req, buff));
    req.isWrite = 0;
    req.kernelID ++;
    assert(0 == device.doRequestSync(&req, cmpbuff));
    assert(std::memcmp(buff, cmpbuff, DataLength) == 0);
    ELOG_INFO("caseReadWriteReadOnce %s",green("ok").c_str());
}


void caseReadWriteReadPartOne()
{
    ELOG_INFO("caseReadWriteReadPartOne %s",yellow("started").c_str());
    struct EBRequest req;

    req.isWrite = 0; //读写标志
    req.length = DataLength;   //请求的字节数
    req.kernelID = 1;

    req.off = 0;

    char buff[DataLength] , cmpbuff[DataLength];
    testFillBuff(reinterpret_cast<unsigned char *>(buff), DataLength);

    assert( 0 == device.doRequestSync(&req, buff));
    req.isWrite = 1;
    req.kernelID ++;
    assert(0 == device.doRequestSync(&req, buff));
    req.isWrite = 0;
    req.length /= 2;
    req.kernelID ++;
    assert(0 == device.doRequestSync(&req, cmpbuff));
    assert(std::memcmp(buff, cmpbuff, DataLength/2) == 0);
    ELOG_INFO("caseReadWriteReadPartOne %s",green("ok").c_str());
}


void caseReadWriteReadPartTwo()
{
    ELOG_INFO("caseReadWriteReadPartTwo %s",yellow("started").c_str());
    struct EBRequest req;

    req.isWrite = 0; //读写标志
    req.length = DataLength;   //请求的字节数
    req.kernelID = 1;

    req.off = 0;

    char buff[DataLength] , cmpbuff[DataLength];
    testFillBuff(reinterpret_cast<unsigned char *>(buff), DataLength);

    assert( 0 == device.doRequestSync(&req, buff));
    req.isWrite = 1;
    req.kernelID ++;
    assert(0 == device.doRequestSync(&req, buff));
    req.isWrite = 0;
    req.length /= 2;
    req.off = req.length;
    req.kernelID ++;
    assert(0 == device.doRequestSync(&req, cmpbuff));
    assert(std::memcmp(buff + req.length, cmpbuff, DataLength/2) == 0);
    ELOG_INFO("caseReadWriteReadPartTwo %s",green("ok").c_str());
}

void caseWriteReadPartThree()
{
    ELOG_INFO("caseWriteReadPartThree %s",yellow("started").c_str());
    struct EBRequest req;

    req.isWrite = 0; //读写标志
    req.length = DataLength;   //请求的字节数
    req.kernelID = 1;

    req.off = 0;

    char buff[DataLength] , cmpbuff[DataLength];
    testFillBuff(reinterpret_cast<unsigned char *>(buff), DataLength);

    assert( 0 == device.doRequestSync(&req, buff));
    req.isWrite = 1;
    req.kernelID ++;
    assert(0 == device.doRequestSync(&req, buff));
    req.isWrite = 0;
    req.length /= 2;
    req.kernelID ++;
    req.off = req.length/2;
    assert(0 == device.doRequestSync(&req, cmpbuff));
    assert(std::memcmp(buff + req.length/2, cmpbuff, DataLength/2) == 0);
    ELOG_INFO("caseWriteReadPartThree %s",green("ok").c_str());
}

//两个写凑城buff用不同分叉的两次读读取
void caseTwoWriteTwoReadOne()
{
    ELOG_INFO("caseTwoWriteTwoReadOne %s",yellow("started").c_str());
    struct EBRequest req;

    req.isWrite = 0; //读写标志
    req.length = DataLength;   //请求的字节数
    req.kernelID = 1;

    req.off = 0;
    char buff[DataLength] , cmpbuff[DataLength];
    testFillBuff(reinterpret_cast<unsigned char *>(buff), DataLength);

    assert( 0 == device.doRequestSync(&req, buff));
    req.kernelID++;
    req.isWrite = 1;
    req.length /= 2;
    assert(0 == device.doRequestSync(&req, buff));

    req.off =  req.length;
    req.kernelID++;
    assert( 0 == device.doRequestSync(&req, buff + req.off));

    req.isWrite = 0;
    req.kernelID++;
    req.off =0;
    req.length = DataLength/4;
    assert(0 == device.doRequestSync(&req, cmpbuff));
    assert(std::memcmp(buff, cmpbuff, req.length) == 0);
    req.kernelID++;
    req.off = req.length;
    req.length = DataLength/4*3;
    assert(0 == device.doRequestSync(&req, cmpbuff  + req.off ));

    assert(std::memcmp(buff, cmpbuff, DataLength) == 0);
    ELOG_INFO("caseTwoWriteTwoReadOne %s",green("ok").c_str());
}


//两个写凑城buff用不同分叉的两次读读取
void caseFiveWritethreeReadOne()
{
    ELOG_INFO("caseFiveWritethreeReadOne %s",yellow("started").c_str());
    struct EBRequest req;

    req.isWrite = 0; //读写标志
    req.length = DataLength;   //请求的字节数
    req.kernelID = 1;

    req.off = 0;
    char buff[DataLength] , cmpbuff[DataLength];
    testFillBuff(reinterpret_cast<unsigned char *>(buff), DataLength);

    for(int i = 0;i < 5; i++)
    {
        req.isWrite = 1;
        req.length = DataLength / 5;
        req.off = i * req.length;
        assert(0 == device.doRequestSync(&req, buff + req.off));
        req.kernelID++;
    }

    for(int i = 0;i < 3; i++)
    {
        req.isWrite = 0;
        req.length = DataLength / 3;
        req.off = i * req.length;
        assert(0 == device.doRequestSync(&req, cmpbuff+ req.off));
        req.kernelID++;
    }
    assert(std::memcmp(buff, cmpbuff, DataLength) == 0);
    ELOG_INFO("caseFiveWritethreeReadOne %s",green("ok").c_str());
}


std::string getProgramDirectory(const char* argv0) {
    char buffer[PATH_MAX];
    // 拷贝argv0到缓冲区
    std::strncpy(buffer, argv0, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';
    // 获取目录名
    std::string directory = dirname(buffer);
    return directory;
}

int main(int argc, const char * argv[])
{
    BaseDir = getProgramDirectory(argv[0]);

    initColor();

    ELOG_INFO("exepath:[%s]", programDir.c_str());

    ELOG_INFO("Test %s", yellow("started ").c_str());

    caseTwoWriteTwoReadOne();

    caseWriteReadPartThree();

    caseReadWriteReadPartTwo();

    caseReadWriteReadPartOne();

    caseReadWriteReadOnce();

    caseWriteReadOnce();

    caseFiveWritethreeReadOne();
//    device.readSync()
    ELOG_INFO("ALL tests %s", green("ok").c_str());
    return 0;
}