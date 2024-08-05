cmake_minimum_required(VERSION 3.10)
project(EBlock C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# 设置AWS SDK的路径，确保指向正确的安装路径
set(AWS_SDK_PATH "/usr/local")

# 包含AWS SDK的CMake模块路径
list(APPEND CMAKE_MODULE_PATH "${AWS_SDK_PATH}/lib64/cmake/AWSSDK")

add_library(Ecolor server/common/EColor.cpp server/common/EColor.h)

add_library(Elog
        server/Log4Eblock.h
        server/Log4Eblock.c)

if(UNIX)
    # 添加C库并链接到C++可执行文件
    add_library(c_functions
            server/EBlockIoctl.c
            server/EBlockIoctl.h
            include/EBLockDeviceCtrl.h
            server/EblockErrors.h
            server/unitTest/TestPendingRequestsLock.cpp
            server/unitTest/TestPendingRequestsLock.h
            server/common/EBlockTypes.h
    )

    add_executable(EblockServer server/EBlockServer.cpp server/EBlockServer.h
            include/EBLockDeviceCtrl.h
            server/EblockErrors.h
            server/EBlockIoctl.h
            server/IODispatch.cpp
            server/IODispatch.h
            server/common/EBlockThreads.hpp
            server/IOTask.cpp server/IOTask.h
            server/S3Client.cpp server/S3Client.h
            server/EBlockDevices.cpp server/EBlockDevices.h
            server/FileBlocks.cpp server/FileBlocks.h
            server/EBlockRangeMap.cpp server/EBlockRangeMap.h
            server/CacheDevice.cpp server/CacheDevice.h
            server/PendingRequestsLock.cpp
            server/PendingRequestsLock.h
            server/common/EBlockTypes.h
            server/common/EBlockRequestsCommon.cpp server/common/EBlockRequestsCommon.h
            server/unitTest/FakeClient.cpp server/unitTest/FakeClient.h
    )

#    target_compile_definitions(EblockServer PRIVATE LOCAL_DEIVCE=1)

    # 查找AWS SDK包
    find_package(AWSSDK REQUIRED COMPONENTS s3)
    target_link_libraries(EblockServer c_functions Ecolor Elog ${AWSSDK_LINK_LIBRARIES})
endif()

############################################# 以下为单元测试 #########################################
add_library(testlib  server/unitTest/PendingIDs.cpp
        server/unitTest/PendingIDs.h
        server/unitTest/SafeCounter.cpp
        server/unitTest/SafeCounter.h
        server/unitTest/FakeClient.cpp
        server/unitTest/FakeClient.h
        server/common/EBlockRequestsCommon.cpp
        server/common/EBlockRequestsCommon.h
        server/unitTest/XorTag.cpp
        server/unitTest/XorTag.h
)
#unit test For PendingRquestLock
add_library(lock_function server/PendingRequestsLock.h server/PendingRequestsLock.cpp)
add_executable(TestPendingRequestsLock
        server/PendingRequestsLock.h
        server/common/EBlockTypes.h
        server/unitTest/TestPendingRequestsLock.cpp server/unitTest/TestPendingRequestsLock.h
)
target_link_libraries(TestPendingRequestsLock lock_function testlib Ecolor Elog)
#unit test For FileBLocks

add_library(fileblocks_function server/FileBlocks.h server/FileBlocks.cpp)

add_executable(TestFileBlocks server/unitTest/TestFileBlocks.cpp server/unitTest/TestFileBlocks.h
)
target_link_libraries(TestFileBlocks fileblocks_function testlib Ecolor Elog)

add_executable(TestCacheDevice server/unitTest/TestCacheDevice.cpp server/CacheDevice.h server/CacheDevice.cpp
                server/EBlockRangeMap.h server/EBlockRangeMap.cpp
                server/EBloclVirtualDevice.h server/EBloclVirtualDevice.cpp
                server/PendingRequestsLock.h server/PendingRequestsLock.cpp
                server/common/EBlockTypes.h
)
target_compile_definitions(TestCacheDevice PRIVATE LOCAL_DEIVCE=1)
target_link_libraries(TestCacheDevice testlib Ecolor Elog fileblocks_function)