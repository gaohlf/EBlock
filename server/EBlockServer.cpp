//
// Created by 20075 on 2024/6/4.
//

#include "EBlockServer.h"
#include "EBlockIoctl.h"
#include "EblockErrors.h"
#include "IODispatch.h"
#include "common/EBlockThreads.hpp"
#include "S3Client.h"
#include "Log4Eblock.h"
#include "EBlockDevices.h"

#define MAXIOTRHEADS 64

EBlockThreads taskThreads(MAXIOTRHEADS);
EBlockDevices devices;

int main(){
//    S3Client client;
//    client.showListObjects();
//    char buff[100] = "hello world";
//    char newbuff[100] = "";
//    bool success = client.putObject("test", 100, buff);
//    if(!success)
//    {
//        ELOG("client.putObject failed\n");
//    }
//
//    success = client.getObject("test", 0, 100, newbuff);
//    if(!success)
//    {
//        ELOG("client.getObject failed\n");
//    }
//    ELOG("newbuff [%s]", newbuff);
    IODispatch ioDispatch;
    ioDispatch.startWork();
    return 0;
}
