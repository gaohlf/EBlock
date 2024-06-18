//
// Created by 20075 on 2024/6/11.
//

#ifndef EBLOCK_IODISPATCH_H
#define EBLOCK_IODISPATCH_H

#include <iostream>
#include <thread>

class IODispatch {
public:
    void startWork();
private:
    void threadSelectFromEblock();

};


#endif //EBLOCK_IODISPATCH_H
