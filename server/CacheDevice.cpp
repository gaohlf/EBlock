//
// Created by 20075 on 2024/6/21.
//

#include "CacheDevice.h"
#ifdef LOCAL_DEIVCE
#include "unitTest/FakeClient.h"
#else
#include "S3Client.h"
#endif

#include "EblockErrors.h"
#include "common/EBlockRequestsCommon.h"
#include <cassert>
#ifdef LOCAL_DEIVCE
FakeClient client;
#else
S3Client client;//在client中已经包含bucekt信息
#endif

int CacheDevice::doRequestSync(struct EBRequest * request, void *buff)
{
    ELOG_INFO("req:%s", toString(*request).c_str());
    if(request->isWrite)
    {
        return writeSync(request, buff);
    }
    else
    {
        return readSync(request, buff);
    }
}

int CacheDevice::writeSync(struct EBRequest * request, void *buff)
{
    ELOG_INFO("req:%s", toString(*request).c_str());
    std::set<FileBlockDesc> blocks;
    //从cache元数据获取写入请求符号
    int ret = blockRangeMap.allocWriteFileDesc(*request, blocks);
    if(ret != EBLOCK_ERROR_TYPE_OK)
    {
        ELOG_ERROR("ret = %d" ,ret);
        return ret;
    }

    //遍历写入多个文件
    unsigned long writenBytes = 0;
    char * thisBuff =(char *)buff;
    for(auto fileIter = blocks.begin(); fileIter != blocks.end(); ++fileIter)
    {
        std::string filename = fileIter->file.fileName();
        unsigned long len = fileIter->file.size();

        bool success = client.putObject(filename, len, thisBuff);
        if(!success)
        {
            ret = -EBLOCK_ERROR_TYPE_WRITE_FAILED;
            ELOG_ERROR("ret = %d" ,ret);
        }
        thisBuff += len;
        writenBytes += len;
        assert(writenBytes <= request->length);
    }
    //若其中部分文件没有写入成功则整个IO认为写入失败 会馆操作
    if(ret != EBLOCK_ERROR_TYPE_OK)
    {
        return blockRangeMap.revertFileDesc(*request, blocks);
    }
    return blockRangeMap.commitFileDesc(*request, blocks);
}

int CacheDevice::readSync(struct EBRequest * request, void *buff)
{
    std::set<FileBlockDesc> blocks;
    ELOG_INFO("req:%s", toString(*request).c_str());
    //从cache元数据获取写入请求符号
    int ret = blockRangeMap.allocReadFileDesc(*request, blocks);
    ELOG_INFO("req:%s ret=%d",toString(*request).c_str(), ret);
    if(ret != EBLOCK_ERROR_TYPE_OK)
    {
        ELOG_ERROR("req:%s ret=%d",toString(*request).c_str(), ret);
        return ret;
    }

    //遍历写入多个文件
    unsigned long readBytes = 0;
    char * thisBuff = nullptr;

    for(auto fileIter = blocks.begin(); fileIter != blocks.end(); ++fileIter)
    {
        ELOG_INFO("req:[%s] seg = [%s]",toString(*request).c_str(),fileIter->toString().c_str());
        std::string filename = fileIter->file.fileName();
        unsigned long len = fileIter->dataSizeInFile;
        unsigned long off = fileIter->dataOffInFile;

        unsigned long logicalStart = fileIter->logicalStart();
        ELOG_DEBUG("\nrequest->off %lu\n"
                     "logicalStart %lu\n",
                     request->off, logicalStart);
        int off_inreq =  logicalStart - request->off;
        thisBuff = (char *)buff + off_inreq;

        bool success = client.getObject(filename, off, len, thisBuff);
        if(!success)
        {
            ELOG_ERROR("req:[%s] seg = [%s] ret=%d",toString(*request).c_str(),fileIter->toString().c_str(), -EBLOCK_ERROR_TYPE_READ_FAILED);
            return -EBLOCK_ERROR_TYPE_READ_FAILED;
        }
        readBytes += len;
        assert(readBytes <= request->length);
    }

    if(blocks.empty())
    {
        ELOG_ERROR("req:%s ret=%d",toString(*request).c_str(), EBLOCK_ERROR_TYPE_OK);
    }

    ret = blockRangeMap.commitFileDesc(*request, blocks);
    return ret;
}
