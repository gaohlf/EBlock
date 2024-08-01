//
// Created by 20075 on 2024/7/30.
//

#include "EBlockRequestsCommon.h"

std::string toString(EBRequest &req) {
    std::string res = "{devName:\"" + std::string(req.devName)+ "\"," + "isWrite:" + std::to_string(req.isWrite) +
            ",length:" + std::to_string(req.length) + ",off:" + std::to_string(req.off)+
            ",kernelID:" + std::to_string(req.kernelID) + "}";
    return res;
}
