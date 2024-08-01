//
// Created by 20075 on 2024/7/25.
//

#include "FakeClient.h"
#include <iostream>
#include <fstream>
std::string BaseDir = ".";
FakeClient::FakeClient() {

}

FakeClient::~FakeClient() {

}

void FakeClient::showListObjects(void) {
}

bool FakeClient::putObject(std::string name, unsigned long length, const char *buff) {
    std::ofstream file(BaseDir + "/" + std::string(name), std::ios::binary);

    if (!file) {
        return false;
    }

    // 写入数据到文件
    file.write(buff, length);

    // 关闭文件流
    file.close();

    // 检查写入是否成功
    if (!file) {
        return false;
    }
    return true;
}

bool FakeClient::getObject(std::string name, unsigned long off, unsigned long length, char *buff) {
    // 打开文件输入流，以二进制模式读取
    std::ifstream file(BaseDir + "/" + std::string(name), std::ios::binary);

    // 检查文件是否成功打开
    if (!file) {
        return false;
    }

    // 移动文件指针到指定偏移量
    file.seekg(off, std::ios::beg);

    // 检查seekg是否成功
    if (!file) {
        return false;
    }

    // 从文件中读取数据到缓冲区
    file.read(buff, length);

    // 检查读取是否成功
    if (!file) {
        return false;
    }
    // 关闭文件流
    file.close();
    return true;
}
