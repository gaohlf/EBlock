//
// Created by 20075 on 2024/7/8.
//

#include "SafeCounter.h"

long SafeCounter::increase() {
    std::lock_guard<std::mutex> lck(m);
    return ++counter;
}
