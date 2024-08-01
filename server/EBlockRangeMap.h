//
// Created by 20075 on 2024/6/21.
//
#ifndef EBLOCK_EBLOCKRANGEMAP_H
#define EBLOCK_EBLOCKRANGEMAP_H
#include <string>
#include "FileBlocks.h"
#include "common/EBlockTypes.h"
#include "PendingRequestsLock.h"

#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif

#define CACHE_FILE_PREFIX "cache"
#define COLD_FILE_PREFIX  "cold"

/**
 * EBlockRangeMap根据offset和len找到对应文件处理
 * 在最后实现时，底层先实现按照64M分块, 上层再利用redis实现条带锁，
 * 再上层根据写入内容原始大小写入文件，实现Garbage Collection。
 *
 * 1、cahce文件的格式为
 * {CACHE_FILE_PREFIX}/{offset}/{len}/预留版本 默认 0
 * 2、cold的文件格式为
 * {COLD_FILE_PREFIX}/{offset}/{len}/预留版本 默认 0
 * 其中cache的大小看写入IO时合并的大小，cold的大小为程序指定，定义在FileBlock.h的 MAX_FILE_BLOCK_BYTES
 *
 * */
//FIXME: 目前只按照固定文件块区分
//FIXME: 目前暂时不考虑这个类的持久化
//FIXME: 暂不实现启动时检查 deadFiles检查中是否存在activeFiles的文件
class EBlockRangeMap {
private:
    /**
     * 再allocXXXFileDesc时上锁，再commit时解锁，锁针对范围上读写锁
     * */
    PendingRequestsLock requestsLock;
    /**
     * activeFiles中deadFiles数据格式完全相同，根据写入时间可以完成扫描
     * */
    ActiveFileBlocks activeFiles;//近期写入的数据块， 未被整理
    DeadFiles deadFiles;//数据已经被新写入的完全覆盖的文件 Garbage Collection 时删除该文件
    //针对activeFiles 和 deadFiles事务操作的锁
    std::mutex blocksLock;

    /**
     * collectFile的数据格式不同，甚至桶也可以不同 是过了热度的历史数据块,通过数据整理 Garbage Collection
     * */
    //FIXME:暂不实现Garbage Collection和 从collectFiles读取的逻辑
//    FileBlocks collectFiles;//经过数据整理的数据块 比如 64M 按照数据内容进行了切割
private:
    std::string s3Filename(IN struct EBRequest &request);

public:
    //根据写入情况分配文件块
    int allocWriteFileDesc(IN struct EBRequest &request,std::set<FileBlockDesc>  &fileBlocklist);

    //根据写入情况分配文件块
    int allocReadFileDesc(IN struct EBRequest &request,OUT std::set<FileBlockDesc>  &fileBlocklist);

    //根据读写确认释放 缓存区间所和条带锁 相应文件区间锁以及条带锁
    int commitFileDesc(IN struct EBRequest &request,std::set<FileBlockDesc>  &fileBlocklist);

    //写入失败时释放读写锁，但不对数据内容进行提交，
    int revertFileDesc(IN struct EBRequest &request,std::set<FileBlockDesc>  &fileBlocklist);
public:
    //FIXME:实现GC时要上IO范围锁
};

#endif //EBLOCK_EBLOCKRANGEMAP_H
