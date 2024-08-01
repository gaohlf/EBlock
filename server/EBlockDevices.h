//
// Created by 20075 on 2024/6/20.
//

#ifndef EBLOCK_EBLOCKDEVICES_H
#define EBLOCK_EBLOCKDEVICES_H

#include "EBloclVirtualDevice.h"
#include <string>

class EBlockDevices {
public:
    //根据device名称
    EBloclVirtualDevice * deviceByName(std::string devname);
};


#endif //EBLOCK_S3DEVICES_H
