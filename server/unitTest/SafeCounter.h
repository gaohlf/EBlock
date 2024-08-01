//
// Created by 20075 on 2024/7/8.
//

#ifndef EBLOCK_SAFECOUNTER_H
#define EBLOCK_SAFECOUNTER_H


#include <mutex>

class SafeCounter {
    std::mutex m;
    long counter = 0;
public:
    long increase();

    //返回最终判断结果
    long result(){return counter;}

    //重置
    void reset(){counter = 0;}
};


#endif //EBLOCK_SAFECOUNTER_H
