//
// Created by 20075 on 2024/6/21.
//

#ifndef EBLOCK_FILEBLOCKS_H
#define EBLOCK_FILEBLOCKS_H

#include <string>
#include <set>
#include <list>
#include <mutex>
#include <map>
#include <iostream>
#include "Log4Eblock.h"


#define MAX_FILE_BLOCK_MBS   64
#define MAX_FILE_BLOCK_KBS   (MAX_FILE_BLOCK_MBS<<10)
#define MAX_FILE_BLOCK_BYTES (MAX_FILE_BLOCK_KBS<<10)

class FileBlockDesc;
//产生的一个文件结构 FileBlockDesc
class FileDesc
{
    std::string name;//用于存储块的文件
    unsigned long offInBlock;//在整个数据里的大小
    unsigned long fileSize;//文件原始大小
    //只在FileBlocks 中起作用 所以复制时不 进行复制
public:
    std::set<FileBlockDesc *> refBlocks;//引用这个文件的blocks
public:
    FileDesc() = default;

    FileDesc(const std::string &filename, unsigned long off, unsigned long len) :
        name(filename), offInBlock(off), fileSize(len)
    {}

    FileDesc(const FileDesc & other) : name(other.name), offInBlock(other.offInBlock), fileSize(other.fileSize)
    {
        for(auto & fileblockP : other.refBlocks)
        {
            refBlocks.insert(fileblockP);
        }
    }

    std::string toString() const
    {
        return "name:'" + name + "',offInBlock:" + std::to_string(offInBlock) +  ",fileSize:"
            + std::to_string(fileSize) + ",ref:" + std::to_string(refBlocks.size());
    }

    bool conflictWith(const FileDesc & other) const //与另一个file有范围冲突
    {
        if(offInBlock >= other.offInBlock + other.size() || other.offInBlock >= offInBlock +size()) return false;
        return true;
    }

    void ref(FileBlockDesc * block){
        refBlocks.insert(block);
    }

    //if empty return true
    bool unref(FileBlockDesc * block)
    {
        refBlocks.erase(block);
        return refBlocks.empty();
    }

    std::string fileName() const {return name;}

    unsigned long off() const {return this->offInBlock;}

    unsigned long size() const {return fileSize;}

    FileDesc &operator =(const FileDesc & other)
    {
        this->name = other.name;
        this->offInBlock = other.offInBlock;
        this->fileSize = other.fileSize;
        for(auto & fileblockP : other.refBlocks)
        {
            refBlocks.insert(fileblockP);
        }
        return *this;
    }

    bool operator < (const FileDesc & other)const{
        if(this->offInBlock < other.offInBlock) return true;
        if(this->offInBlock >  other.offInBlock) return false;
        // ==
        if(this->fileSize < other.fileSize) return true;
        return false;
    }

    bool operator ==(const FileDesc & other) const{
        return this->offInBlock == other.offInBlock && this->fileSize == other.fileSize;
    }

    bool operator > (const FileDesc & other) const{
        if(this->operator<(other)) return false;
        if(this->operator==(other)) return false;
        return true;
    }

    bool operator <= (const FileDesc & other)const {
        return !this->operator>(other);
    }

    bool operator >=(const FileDesc & other)const {
        return !this->operator<(other);
    }
};

//变成可以被map查询的类
class FileBlockDesc
{
public:
    FileDesc file;//这个file只是用作描述 并不直接与files一一对应
    /**dataOffInFile 相对于FileBlockDesc当中在读取数据时 代表文件内的便宜，在FileBlocks内部代表 有效数据价值*/
    unsigned long dataOffInFile;//有效数据偏倚在
    unsigned long dataSizeInFile;//有效数据长度
public:
    FileBlockDesc() = default;
    //写入一个新的文件时产生 一个FileBlockDesc 初始为文件全范围为最新的数据
    FileBlockDesc(const std::string &filename, unsigned long off, unsigned long len) : file(filename, off, len)
    {
        this->dataOffInFile = 0;
        this->dataSizeInFile = len;
    }

    FileBlockDesc(const FileBlockDesc & other): file(other.file), dataOffInFile(other.dataOffInFile),
        dataSizeInFile(other.dataSizeInFile)
    {}

    unsigned long logicalStart() const
    {
        return this->file.off() +dataOffInFile;
    }

    unsigned long logicalEnd() const
    {
        return logicalStart() + dataSizeInFile;
    }

    bool inSameLogicalRange(const FileBlockDesc & other) const
    {
        return logicalStart() == other.logicalStart() && dataSizeInFile == other.dataSizeInFile;
    }

    std::string toString() const
    {
        return file.toString() + ",dataOffInFile:" + std::to_string(dataOffInFile)
        + ",dataSizeInFile:" + std::to_string(dataSizeInFile);
    }

    //自己有效数据范围是否全面覆盖对方
    bool effFullCover(const FileBlockDesc &other) const
    {
        return (logicalStart() <= other.logicalStart() &&
           logicalEnd() >= other.logicalEnd()
        );
    }

    //存在逻辑交叉
    bool conflictWith(const FileBlockDesc & other) const //与另一个file有范围冲突
    {
        if(file.off() + dataOffInFile >= other.file.off() + other.dataOffInFile + other.dataSizeInFile
        || other.file.off() + other.dataOffInFile >= file.off() + dataOffInFile + dataSizeInFile)
            return false;
        return true;
    }

    //判断是否存在逻辑交叉，若存返回逻辑交叉的部分
    bool conflictPartBaseOnOther(const FileBlockDesc & other, FileBlockDesc & newBlock) const
    {
       if(!this->conflictWith(other)) return false;
        newBlock = other;
        unsigned long thisLogicalOff = this->file.off() + this->dataOffInFile;
        unsigned long thisLogicalEnd = thisLogicalOff + this->dataSizeInFile;
        unsigned long otherLogicalOff = other.file.off() + other.dataOffInFile;
        unsigned long otherLogicalEnd = otherLogicalOff + other.dataSizeInFile;
        unsigned long newLogicalEnd = std::min(otherLogicalEnd, thisLogicalEnd);
        unsigned long newlogicalOff =  std::max(thisLogicalOff, otherLogicalOff);
        ELOG_DEBUG("thisLogicalOff:[%lu] thisLogicalEnd:[%lu] otherLogicalOff:[%lu] otherLogicalEnd:[%lu] newLogicalEnd:[%lu]"
                   "newlogicalOff:[%lu]",
                   thisLogicalOff,thisLogicalEnd,otherLogicalOff, otherLogicalEnd, newLogicalEnd, newlogicalOff);
        newBlock.dataOffInFile = newlogicalOff - newBlock.file.off();
        newBlock.dataSizeInFile = newLogicalEnd - newlogicalOff;
        return true;
    }

    FileBlockDesc &operator =(const FileBlockDesc & other)
    {
        this->file = other.file;
        this->dataOffInFile = other.dataOffInFile;
        this->dataSizeInFile = other.dataSizeInFile;
        return *this;
    }

    bool operator < (const FileBlockDesc & other)const{
        ELOG_DEBUG("%s < %s", this->toString().c_str(), other.toString().c_str());
        if(this->logicalStart() < other.logicalStart()){
            ELOG_DEBUG("logicalStart:%lu"
                       " other.logicalStart:%lu",
                       this->logicalStart(),
                       other.logicalStart());
            return true;
        }
        if(this->logicalStart() > other.logicalStart())
        {
            ELOG_DEBUG("logicalStart:%lu"
                       " other.logicalStart:%lu",
                       this->logicalStart(),
                       other.logicalStart());
            return false;
        }

        if(this->dataSizeInFile < other.dataSizeInFile)
        {
            ELOG_DEBUG("dataSizeInFile:%lu"
                       " other.dataSizeInFile:%lu",
                       this->dataSizeInFile,
                       other.dataSizeInFile);
            return true;
        }

        if(this->dataSizeInFile > other.dataSizeInFile)
        {
            ELOG_DEBUG("dataSizeInFile:%lu"
                       " other.dataSizeInFile:%lu",
                       this->dataSizeInFile,
                       other.dataSizeInFile);
            return false;
        }
        ELOG_DEBUG("file:%s"
                   " other.file:%s",
                   this->file.toString().c_str(),
                   other.file.toString().c_str());
        return file < other.file;
    }

    bool operator ==(const FileBlockDesc & other) const{
        return this->logicalStart() == other.logicalStart() &&
               this->logicalEnd() == other.logicalEnd() &&
               this->file == other.file;
    }

    bool operator > (const FileBlockDesc & other) const{
        if(this->operator<(other)) return false;
        if(this->operator==(other)) return false;
        return true;
    }

    bool operator <= (const FileBlockDesc & other)const {
        return !this->operator>(other);
    }

    bool operator >=(const FileBlockDesc & other)const {
        return !this->operator<(other);
    }
};

//外部上事务锁，本类中无并发保护
class ActiveFileBlocks {
public:
    //文件集合 产生的文件被多次引用时
    std::map<FileDesc, FileDesc> files;
    //文件块一个filedesc代表一个逻辑逻辑读写段
    std::map<FileBlockDesc, FileBlockDesc> fileblocks;

private:
    void tryRemoveEffOldBlocks(const FileBlockDesc &newBlock, std::set<FileDesc> &ToRemovefiles);

    void tryAddNewBlock(const FileBlockDesc &newBlock);

    void findEffectiveFiles(const FileBlockDesc &newBlock, std::set<FileDesc> &effectiveFiles);

    void removeOldBlock(FileBlockDesc *oldBlock,std::set<FileDesc> &ToRemovefiles);

    void splitOldBlock(FileBlockDesc *oldBlock, const FileBlockDesc &newBlock,std::map<FileBlockDesc ,FileBlockDesc> &toChangeBlocks);

    void changeOldBlock(FileBlockDesc *oldBlock, const FileBlockDesc &newBlock,std::map<FileBlockDesc ,FileBlockDesc> &toChangeBlocks);

    //插新的一个文件，改变覆盖范围的block 并改变覆盖范围内的读写请求 从列表中移除 已经废弃的文件
    void CoverFile(const FileBlockDesc &newBlock, std::set<FileDesc> &ToRemovefiles);

    //检查两种形式的逻辑方位是否相同
    bool compareLogical(const FileBlockDesc &inputDesc, const std::set<FileBlockDesc> &newBlocks);

    void readdOldBLock(const FileBlockDesc &oldBlock, const FileBlockDesc &changedBlock);
public:
    //输入为要读取时覆盖的逻辑范围，输出为从列表中获取的真实的数据值的总和  返回值为是否能够全部覆盖
    bool allInFileBlocks(const FileBlockDesc &inputDesc, std::set<FileBlockDesc> &newBlocks);

    //插新的一组文件，改变覆盖范围的block 并改变覆盖范围内的读写请求 从列表中移除 已经废弃的文件
    void CoverFiles(const std::set<FileBlockDesc> &newBlocks, std::set<FileDesc> &ToRemovefiles);

    std::string toString() const
    {
        std::string result = "{files:[";
        for(auto file: files)
        {
            result += "{" +file.second.toString()+ "},";
        }
        result += "],fileblocks:[";
        for(auto fileblock: fileblocks)
        {
            result += "{" +fileblock.second.toString()+ "},";
        }
        result += "]}";
        return result;
    }

};

//用于存储
class DeadFiles
{
    std::set<FileDesc> files;
private:
    //判断是在files已经存在相同的文件片段
    bool haveSameFiles(std::set<FileDesc> &ToRemovefiles);
public:
    //将新的结构插入到尾部 用于删除
    void AddFileTail(std::set<FileDesc> &ToRemovefiles);

    //如果写入产生一个待删除的则需要从待删除文件中删除他
    void checkRemove(std::set<FileBlockDesc> &newBlocks);
};

#endif //EBLOCK_FILEBLOCKS_H
