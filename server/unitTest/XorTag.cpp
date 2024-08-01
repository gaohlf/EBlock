//
// Created by 20075 on 2024/8/1.
//

#include "XorTag.h"


//快速计算一个buff的特征值
char checkTag(unsigned char * buff, unsigned long length)
{
    char res = 0;
    for(auto i = 0;i < length; ++i)
    {
        res ^= buff[i];
    }
    return res;
}