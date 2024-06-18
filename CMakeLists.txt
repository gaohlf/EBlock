cmake_minimum_required(VERSION 3.10)
project(EBlock C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# 添加C库并链接到C++可执行文件
add_library(c_functions
        server/EBlockIoctl.c
        server/EBlockIoctl.h
        include/EBLockDeviceCtrl.h
        server/EblockErrors.h
        server/Log4Eblock.h
        server/Log4Eblock.c
)

add_executable(EblockServer server/EBlockServer.cpp server/EBlockServer.h
        include/EBLockDeviceCtrl.h
        server/EblockErrors.h
        server/EBlockIoctl.h
        server/IODispatch.cpp
        server/IODispatch.h
        server/Log4Eblock.h
)

target_link_libraries(EblockServer c_functions)