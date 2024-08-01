//
// Created by 20075 on 2024/6/21.
//

#include "FileBlocks.h"
#include <cassert>
#include <iostream>

//检查两种形式的逻辑方位是否相同
bool ActiveFileBlocks::compareLogical(const FileBlockDesc &inputDesc, const std::set<FileBlockDesc> &newBlocks)
{
    bool isFirst = true;
    unsigned long FirstLogicalOff = 0;
    unsigned long TotalLen = 0;
    unsigned long lastLogicalOff = 0;
    unsigned long lastLogicalLen = 0;
    unsigned long curlogicalOff = 0;
    unsigned long curlogicalLen = 0;
   for(auto block : newBlocks)
    {
        curlogicalOff = block.dataOffInFile + block.file.off();
        curlogicalLen = block.dataSizeInFile;
        TotalLen += curlogicalLen;

        if(isFirst)
        {//首次更新
            FirstLogicalOff = curlogicalOff;
            isFirst = false;
        }
        else
        {
            if(lastLogicalOff + lastLogicalLen != curlogicalOff)
            {
                return false;
            }
        }

        lastLogicalOff = curlogicalOff;
        lastLogicalLen = curlogicalLen;
    }

    if(TotalLen != inputDesc.dataSizeInFile || FirstLogicalOff != inputDesc.dataOffInFile + inputDesc.file.off())
    {
        return false;
    }

    return true;
}

//输入为要读取时覆盖的逻辑范围，输出为从列表中获取的真实的数据值的总和 返回值为是否能够全部覆盖
bool ActiveFileBlocks::allInFileBlocks(const FileBlockDesc &inputDesc, std::set<FileBlockDesc> &newBlocks)
{   //先找到所有的相关文件
    std::set<FileDesc > effectiveFiles;

    findEffectiveFiles(inputDesc, effectiveFiles);

    //2）从有效数据文件中 找到与 inputDesc交的文件块
    for(auto &file : effectiveFiles) {//3)遍历有效文件中文件块
        for (auto oldblock: file.refBlocks)
        {
            FileBlockDesc newBlock;
            if(inputDesc.conflictPartBaseOnOther(*oldblock, newBlock))
            {
                newBlocks.insert(newBlock);
                ELOG_DEBUG("newBlock:%s",newBlock.toString().c_str());
             }
        }
    }
    ELOG_DEBUG("inputDesc:%s",inputDesc.toString().c_str());

    return compareLogical(inputDesc, newBlocks);
}

//FIXME：因为files是顺序排列的 这个可以查找相关file的方式可以优化
void ActiveFileBlocks::findEffectiveFiles(const FileBlockDesc &newBlock, std::set<FileDesc > &effectiveFiles) {
//1) 从files中找到有效数据集与 与 newBlock 的有效数据集合（就是文件集合）相交的文件
    for(auto filepair : files)
    {
        if(filepair.second.conflictWith(newBlock.file))
        {
            effectiveFiles.insert(filepair.second);
        }
    }
}

//将oldblock从文件和set中移除 判断file中无block时加入到ToRemovefiles中
void ActiveFileBlocks::removeOldBlock(FileBlockDesc *oldBlock, std::set<FileDesc> &ToRemovefiles) {
    FileBlockDesc tmpOldBlock = *oldBlock;

    auto it = files.find(oldBlock->file);
    assert(it != files.end());
    if( it->second.unref(oldBlock))
    {
        ToRemovefiles.insert(it->second);
        files.erase(it);
    }

    ELOG_DEBUG("removeOldBlock~~~~~~~~~~~\n"
                 "oldblock:%s\n"
                 "blocks:%s"
    ,tmpOldBlock.toString().c_str(),this->toString().c_str());

    auto blockit = fileblocks.find(tmpOldBlock);
    assert(blockit != fileblocks.end());
    fileblocks.erase(blockit);

    ELOG_DEBUG("~~~~~~~~ BLocks\n %s", this->toString().c_str());
}
//{name:'./test_8_60',offInBlock:8,fileSize:60,ref:0,dataOffInFile:30,dataSizeInFile:30}
//{name:'./test_8_60',offInBlock:8,fileSize:60,ref:0,dataOffInFile:30,dataSizeInFile:30}

//{name:'./test_8_30',offInBlock:8,fileSize:30,ref:0,dataOffInFile:0,dataSizeInFile:30}

//将现有文件块修改为第一段，将文件块后续文件块 插入到 newblock中，ref 文件
void ActiveFileBlocks::splitOldBlock(FileBlockDesc *oldBlock, const FileBlockDesc &newBlock,
                                     std::map<FileBlockDesc ,FileBlockDesc> &toChangeBlocks) {
    FileBlockDesc targetBlock = *oldBlock;
    unsigned long oldLogicalOff = oldBlock->file.off() + oldBlock->dataOffInFile;
    unsigned long newLogicalOff = newBlock.file.off() + newBlock.dataOffInFile;
    unsigned long oldLogicalEnd = oldLogicalOff +  oldBlock->dataSizeInFile;
    unsigned long newLogicalEnd = newLogicalOff + newBlock.dataSizeInFile;
    //old的范围比new的大
    assert(oldLogicalOff <= newLogicalOff && oldLogicalEnd >=newLogicalEnd);
    //两个线段左侧相同，将oldblock改为右侧不相交部分
    if(oldLogicalOff == newLogicalOff)
    {
        targetBlock.dataOffInFile = newLogicalEnd - oldBlock->file.off();
        targetBlock.dataSizeInFile = oldLogicalEnd - newLogicalEnd;
        ELOG_DEBUG("newLogicalOff:%lu\n"
                     "oldLogicalOff:%lu\n"
                     "oldBlock->dataSizeInFile:%lu\n"
                     "block=%s",
                     newLogicalOff, oldLogicalOff, targetBlock.dataSizeInFile,this->toString().c_str());
        toChangeBlocks.insert(std::make_pair(*oldBlock, targetBlock));
        return;
    }

    if(oldLogicalEnd == newLogicalEnd)
    {//右侧相同，oldblock留前半部分
        targetBlock.dataSizeInFile = newLogicalOff - oldLogicalOff;//减去线段的尾部

        ELOG_WARNING("oldLogicalEnd:%lu\n"
                     "newLogicalEnd:%lu\n"
                     "oldBlock->dataSizeInFile:%lu\n",
                     "block=%s",
                     oldLogicalEnd, newLogicalEnd, targetBlock.dataSizeInFile,this->toString().c_str());
        toChangeBlocks.insert(std::make_pair(*oldBlock, targetBlock));
        return;
    }

    //第一段 左侧oldBlock->dataOffInFile 不变
    targetBlock.dataSizeInFile = newLogicalOff - oldLogicalOff;
    ELOG_WARNING("newLogicalOff:%lu\n"
                 "oldLogicalOff:%lu\n"
                 "oldBlock->dataSizeInFile:%lu\n",
                 "block=%s",
                 newLogicalOff, oldLogicalOff, oldBlock->dataSizeInFile,this->toString().c_str());
    //第二段 是old被new切割的右半部分
    FileBlockDesc secondBlock = *oldBlock;
    secondBlock.dataOffInFile = newLogicalEnd - oldBlock->file.off();
    secondBlock.dataSizeInFile = oldLogicalEnd - newLogicalEnd;

    //将新的block插入到fileblocks并找到原来的file增加ref.
    auto blockpair = fileblocks.insert(std::make_pair(secondBlock, secondBlock));
    assert(blockpair.second);
    auto fileBlockIt = blockpair.first;
    //找到file 新引用一个block
    auto it = files.find(secondBlock.file);
    assert(it!= files.end());
    it->second.ref(&fileBlockIt->second);

    toChangeBlocks.insert(std::make_pair(*oldBlock, targetBlock));
}

//改变找到的文件块oldblock的 dataOffInFile 与 dataSizeInFile
void ActiveFileBlocks::changeOldBlock(FileBlockDesc *oldBlock, const FileBlockDesc &newBlock,
                                      std::map<FileBlockDesc ,FileBlockDesc> &toChangeBlocks) {
    FileBlockDesc targetBlock = *oldBlock;
    unsigned long oldLogicalOff = targetBlock.file.off() + oldBlock->dataOffInFile;
    unsigned long newLogicalOff = newBlock.file.off() + newBlock.dataOffInFile;
    unsigned long oldLogicalEnd = oldLogicalOff +  oldBlock->dataSizeInFile;
    unsigned long newLogicalEnd = newLogicalOff + newBlock.dataSizeInFile;
    unsigned long mergeLogicalOff = std::max(oldLogicalOff, newLogicalOff);
    unsigned long mergeLogicalEnd = std::min(oldLogicalEnd, newLogicalEnd);
    unsigned long targetLogicalOff = 0, targetLogicalEnd = 0;

    assert(oldLogicalOff != newLogicalOff && oldLogicalEnd != newLogicalEnd);
    //oldblock的左侧
    if(oldLogicalOff < mergeLogicalOff)
    {
        targetLogicalOff = oldLogicalOff;
        targetLogicalEnd = mergeLogicalOff;
    }
    else
    {
        targetLogicalOff = mergeLogicalEnd;
        targetLogicalEnd = oldLogicalEnd;
    }
    targetBlock.dataOffInFile = targetLogicalOff - targetBlock.file.off();
    targetBlock.dataSizeInFile = targetLogicalEnd - targetLogicalOff;

    toChangeBlocks.insert(std::make_pair(*oldBlock, targetBlock));
}

void ActiveFileBlocks::tryRemoveEffOldBlocks(const FileBlockDesc &newBlock, std::set<FileDesc> &ToRemovefiles) {
    std::set<FileDesc > effectiveFiles;

    findEffectiveFiles(newBlock, effectiveFiles);

    //2）从有效数据文件中 找到与 newblock交的文件块
    for(auto &file : effectiveFiles)
    {//3)遍历有效文件中文件块
        std::map<FileBlockDesc , FileBlockDesc> toChangeBlocks;
        for(auto oldblock : file.refBlocks)
        {
            if(newBlock == *oldblock)
            {//完全一致在什么也不需要做直接返回
                ELOG_DEBUG("newBlock==*oldblock ~~~~~~~~~~~\n"
                             "oldblock:%s\n"
                             "newBlock:%s\n"
                ,oldblock->toString().c_str(),
                             newBlock.toString().c_str());
                return;
            }
            else if(newBlock.effFullCover(*oldblock))
            { //3）判断若newblock覆盖 oldblock的有效范围 则将oldblock从文件和set中移除 判断file中无block时加入到ToRemovefiles中
                ELOG_DEBUG("newBlock.effFullCover(*oldblock)~~~~~~~~~~~\n"
                             "oldblock:%s\n"
                             "newBlock:%s\n"
                             ,oldblock->toString().c_str(),
                             newBlock.toString().c_str());
                removeOldBlock(oldblock, ToRemovefiles);
            }
            else if(oldblock->effFullCover(newBlock))
            {//4）在3）之后排除了相等的情况 若oldblock覆盖 newblock 将现有文件块修改为第一段，将文件块后续文件块 插入到 newblock中，ref 文件
                ELOG_DEBUG("oldblock->effFullCover(newBlock)\n"
                             "oldblock:%s\n"
                             "newBlock:%s\n"
                             ,oldblock->toString().c_str(),
                             newBlock.toString().c_str());
                splitOldBlock(oldblock, newBlock, toChangeBlocks);
                ELOG_DEBUG("oldblock->effFullCover(newBlock)\n"
                             "oldblock:%s\n"
                             "newBlock:%s\n"
                            ,oldblock->toString().c_str(),
                             newBlock.toString().c_str());
            }
            else if(newBlock.conflictWith(*oldblock))
            {//5) 若 两个文件块交叉不覆盖，改变找到的文件块oldblock的 dataOffInFile 与 dataSizeInFile
                ELOG_WARNING("newBlock.conflictWith(*oldblock)\n"
                             "oldblock:%s\n"
                             "newBlock:%s\n"
                ,oldblock->toString().c_str(),
                             newBlock.toString().c_str());
                changeOldBlock(oldblock, newBlock, toChangeBlocks);
                ELOG_WARNING("oldblock->effFullCover(newBlock)\n"
                             "oldblock:%s\n"
                             "newBlock:%s\n"
                             ,oldblock->toString().c_str(),
                             newBlock.toString().c_str());

            }
        }

        for(auto it : toChangeBlocks)
        {
            readdOldBLock(it.first, it.second);
        }
    }
}

void ActiveFileBlocks::readdOldBLock(const FileBlockDesc &oldBlock, const FileBlockDesc &changedBlock)
{
    auto blockpair = fileblocks.insert(std::make_pair(changedBlock, changedBlock));
    assert(blockpair.second);
    //插入成功说明这是一个新的 fileblock
    auto fileBlockIt = blockpair.first;
    //    std::cout << newBlock.file.toString() << std::endl;
    //尝试插入新file 没有成功则会返回已有的block
    auto it = files.find(changedBlock.file);
    //返回对应 等价迭代器的里的 file 增加对 fileblock的引用
    it->second.ref(&fileBlockIt->second);
    auto fileOldBlockIt = fileblocks.find(oldBlock);

    it->second.unref(&fileOldBlockIt->second);
    fileblocks.erase(fileOldBlockIt);
}

void ActiveFileBlocks::tryAddNewBlock(const FileBlockDesc &newBlock) {
    auto blockpair = fileblocks.insert(std::make_pair(newBlock, newBlock));
    if(!blockpair.second)
    {//已经插入什么也不需要做
        return;
    }
    //插入成功说明这是一个新的 fileblock
    auto fileBlockIt = blockpair.first;
    //    std::cout << newBlock.file.toString() << std::endl;
    //尝试插入新file 没有成功则会返回已有的block
    auto filepair = files.insert(std::make_pair(newBlock.file, newBlock.file));
    //返回对应 等价迭代器的里的 file 增加对 fileblock的引用
    filepair.first->second.ref(&fileBlockIt->second);
}


//插新的一个文件，改变覆盖范围的block 并改变覆盖范围内的读写请求 从列表中移除 已经废弃的文件
void ActiveFileBlocks::CoverFile(const FileBlockDesc &newBlock, std::set<FileDesc> &ToRemovefiles)
{
    //尝试移除被新写入覆盖 的可以移除的 块
    tryRemoveEffOldBlocks(newBlock, ToRemovefiles);

    //尝试将新的block 插入到 ActiveBLocks中
    tryAddNewBlock(newBlock);
}


//插新的一组文件，改变覆盖范围的block 并改变覆盖范围内的读写请求 从列表中移除 已经废弃的文件
void ActiveFileBlocks::CoverFiles(const std::set<FileBlockDesc> &newBlocks, std::set<FileDesc> &ToRemovefiles)
{   //NOTE:遍历newblocks的所有片段 事实上在最初实现时，该怕blocks只可能有一个file
    for(auto &file : newBlocks)
    {
        std::set<FileDesc> thisRemoveFiles;
        this->CoverFile(file, thisRemoveFiles);

        for(auto & item : thisRemoveFiles)
        {
            ToRemovefiles.insert(item);
        }
    }
}


bool DeadFiles::haveSameFiles(std::set<FileDesc> &ToRemovefiles)
{
        for (auto & trfiles : ToRemovefiles)
        {
            auto it = files.find(trfiles);
            if(it != files.end())
            {
                return true;
            }
        }
    return false;
}

//将新的结构插入到尾部 用于删除
void DeadFiles::AddFileTail(std::set< FileDesc> &ToRemovefiles)
{
    //NOTE：逻辑上因为再alloc时有 check_remove所以list当中一定不可能有ToRemovefiles
    assert(!haveSameFiles(ToRemovefiles));

    for(auto & trfiles : ToRemovefiles)
    {
        files.insert(trfiles);
    }

    //FIXME：伴随回收一起实现
}

//如果写入产生一个待删除的则需要从待删除文件中删除他
void DeadFiles::checkRemove(std::set<FileBlockDesc> &newBlocks)
{
    //遍历删除newblocks中的所有数据快
    for (auto & file : newBlocks) {
        files.erase(file.file);
    }
}