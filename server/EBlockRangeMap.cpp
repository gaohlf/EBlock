//
// Created by 20075 on 2024/6/21.
//

#include "EBlockRangeMap.h"
#include "EblockErrors.h"
#include "../include/EBlockRequests.h"
#include "common/EBlockRequestsCommon.h"
#include <cassert>
//std::string filename = "cache_" + std::to_string(off)+"_"+std::to_string(len);

std::string EBlockRangeMap::s3Filename(IN struct EBRequest &request)
{
    std::string filename = "cache_" + std::to_string(request.off)+"_"+std::to_string(request.length);
    return filename;
}

/**
 * 写入时将一整个原始IO构造完全下放
 * */
int EBlockRangeMap::allocWriteFileDesc(IN struct EBRequest &request,OUT std::set<FileBlockDesc>  &fileBlocklist)
{
    //IO范围锁
    requestsLock.getLockSync(&request);

    FileBlockDesc desc(s3Filename(request), request.off, request.length);
    fileBlocklist.insert(desc);

    //保护 activeFiles和deadFiles 移动列表操作的锁
    std::lock_guard<std::mutex> lock(blocksLock);

    deadFiles.checkRemove(fileBlocklist);
    return EBLOCK_ERROR_TYPE_OK;
}

//根据写入情况分配文件块
int EBlockRangeMap::allocReadFileDesc(IN struct EBRequest &request, std::set<FileBlockDesc>  &fileBlocklist)
{
    //IO范围锁
    requestsLock.getLockSync(&request);
    FileBlockDesc desc(s3Filename(request), request.off, request.length);
    //保护 activeFiles和deadFiles 移动列表操作的锁
    std::lock_guard<std::mutex> lock(blocksLock);
    bool iscoverd = activeFiles.allInFileBlocks(desc,fileBlocklist);
    if(iscoverd)
    {
        return EBLOCK_ERROR_TYPE_OK;
    }
    //FIXME：暂不考虑从其他区域获取和合并的问题
    return EBLOCK_ERROR_TYPE_OK;
}

//根据读写确认释放 缓存区间锁和条带锁 相应文件区间锁以及条带锁
int EBlockRangeMap::commitFileDesc(IN struct EBRequest &request,IN std::set<FileBlockDesc>  &fileBlocklist)
{
    if(request.isWrite)
    {
        //保护 activeFiles和deadFiles 移动列表操作的锁
        std::lock_guard<std::mutex> lock(blocksLock);

        std::set< FileDesc> ToRemovefiles;
        activeFiles.CoverFiles(fileBlocklist, ToRemovefiles);
        deadFiles.AddFileTail(ToRemovefiles);
    }
    //释放IO锁
    requestsLock.releaseLockSync(&request);
    return EBLOCK_ERROR_TYPE_OK;
}


//根据读写确认释放 缓存区间锁和条带锁 相应文件区间锁以及条带锁
int EBlockRangeMap::revertFileDesc(IN struct EBRequest &request,IN std::set<FileBlockDesc>  &fileBlocklist)
{
    if(request.isWrite)
    {
        //FIXME: 写出错元数据暂不处理
        ELOG_ERROR("abort request = %s" ,toString(request).c_str());
        abort();
        //保护 activeFiles和deadFiles 移动列表操作的锁
        std::lock_guard<std::mutex> lock(blocksLock);

       //FIXME：对有可能产生的中间垃圾 文件片进行处理，如果新写入的在activeFiles，或这deadFiles不做处理，
       // 否则要加入deadFiles
    }
    //释放IO锁
    requestsLock.releaseLockSync(&request);
    return EBLOCK_ERROR_TYPE_OK;
}
