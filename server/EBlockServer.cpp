//
// Created by 20075 on 2024/6/4.
//

#include "EBlockServer.h"
#include "EBlockIoctl.h"
#include "EblockErrors.h"
#include "IODispatch.h"


int main(){
    IODispatch ioDispatch;
    ioDispatch.startWork();
    return 0;
}
