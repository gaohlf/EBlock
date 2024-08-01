//
// Created by 20075 on 2024/6/21.
//

#ifndef EBLOCK_CACHEDEVICE_H
#define EBLOCK_CACHEDEVICE_H

#include "EBloclVirtualDevice.h"
#include "EBlockRangeMap.h"

class CacheDevice :public EBloclVirtualDevice{
private:
    EBlockRangeMap blockRangeMap;
public:
    int writeSync(struct EBRequest * request, void *buff);

    int readSync(struct EBRequest * request, void *buff);
public:
    int doRequestSync(struct EBRequest * request, void *buff) override;
};


#endif //EBLOCK_CACHEDEVICE_H
