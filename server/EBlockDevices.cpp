//
// Created by 20075 on 2024/6/20.
//

#include "EBlockDevices.h"
#include "CacheDevice.h"

//设备
static CacheDevice device;

EBloclVirtualDevice * EBlockDevices::deviceByName(std::string devname)
{
    return &device;
}