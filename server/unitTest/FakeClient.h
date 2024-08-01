//
// Created by 20075 on 2024/7/25.
//

#ifndef EBLOCK_FAKECLIENT_H
#define EBLOCK_FAKECLIENT_H

#include <string>

extern std::string BaseDir;

class FakeClient {
public:
    FakeClient();
    ~FakeClient();

    //列举objects
    void showListObjects(void);

    //上传指定名称对象的指定数据
    bool putObject(std::string name, unsigned long length, const char * buff);

    //下载指定名称对象的指定数据
    bool getObject(std::string name, unsigned long off, unsigned long length, char * buff);
};

#endif //EBLOCK_FAKECLIENT_H
