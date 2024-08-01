//
// Created by 20075 on 2024/6/21.
//

#ifndef EBLOCK_EBLOCLVIRTUALDEVICE_H
#define EBLOCK_EBLOCLVIRTUALDEVICE_H

#include "EBlockIoctl.h"

class EBloclVirtualDevice {
public:
    virtual int doRequestSync(struct EBRequest * request, void *buff) = 0;
};


#endif //EBLOCK_EBLOCLVIRTUALDEVICE_H
