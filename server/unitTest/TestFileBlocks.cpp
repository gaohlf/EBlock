//
// Created by 20075 on 2024/7/9.
//
#include <istream>
#include <set>
#include <iostream>
#include <cassert>
#include "TestFileBlocks.h"
#include "../FileBlocks.h"
#include "../Log4Eblock.h"
#include "../common/EColor.h"

extern std::string green(const std::string & org);

extern std::string yellow(const std::string & org);

extern std::string cyan(const std::string & org);

//与写范围相同的读
void caseSameWriteRange(ActiveFileBlocks &blocks, unsigned long off, unsigned long len, int exprReadSegNum)
{
    ELOG_INFO("  ->caseSameWriteRange %s",cyan("start").c_str());
    FileBlockDesc desc("",off, len);

    std::set<FileBlockDesc> newBlocks;

    ELOG_DEBUG(blocks.toString().c_str());
    ELOG_DEBUG(desc.toString().c_str());
    assert(blocks.allInFileBlocks(desc, newBlocks));

    assert(newBlocks.size() == exprReadSegNum);

    if(exprReadSegNum == 1)
    {
        auto newdesc = newBlocks.begin();
        assert(newdesc != newBlocks.end());
        assert(newBlocks.size() == 1);
        ELOG_DEBUG(newdesc->toString().c_str());
        ELOG_DEBUG(desc.toString().c_str());
        assert(*newdesc == desc);
    }

    ELOG_INFO("  ->caseSameWriteRange %s", green("OK").c_str());
}

void caseSameStartDiffEnd(ActiveFileBlocks &blocks, unsigned long off, unsigned long len, int exprReadSegNum)
{
    ELOG_INFO("  ->caseSameStartDiffEnd %s",cyan("start").c_str());
    FileBlockDesc desc("",off, len-1);

    std::set<FileBlockDesc> newBlocks;

    assert(blocks.allInFileBlocks(desc, newBlocks));

    assert(newBlocks.size() == exprReadSegNum);

    if(exprReadSegNum == 1)
    {
        auto newdesc = newBlocks.begin();
        assert(newdesc != newBlocks.end());
        assert(newBlocks.size() == 1);
        ELOG_DEBUG(newdesc->toString().c_str());
        ELOG_DEBUG(desc.toString().c_str());
        assert(newdesc->inSameLogicalRange(desc));
    }

    ELOG_INFO("  ->caseSameStartDiffEnd %s", green("OK").c_str());
}

void caseDiffStartSameEnd(ActiveFileBlocks &blocks, unsigned long off, unsigned long len, int exprReadSegNum)
{
    ELOG_INFO("  ->caseDiffStartSameEnd %s",cyan("start").c_str());
    FileBlockDesc desc("",off+1, len-1);

    std::set<FileBlockDesc> newBlocks;

    assert(blocks.allInFileBlocks(desc, newBlocks));

    assert(newBlocks.size() == exprReadSegNum);

    if(exprReadSegNum == 1)
    {
        auto newdesc = newBlocks.begin();
        assert(newdesc != newBlocks.end());
        assert(newBlocks.size() == 1);
        ELOG_DEBUG(newdesc->toString().c_str());
        ELOG_DEBUG(desc.toString().c_str());
        assert(newdesc->inSameLogicalRange(desc));
    }

    ELOG_INFO("  ->caseDiffStartSameEnd %s", green("OK").c_str());
}



void caseDiffStartDiffEnd(ActiveFileBlocks &blocks, unsigned long off, unsigned long len, int exprReadSegNum)
{
    ELOG_INFO("  ->caseDiffStartSameEnd %s",cyan("start").c_str());
    FileBlockDesc desc("",off+1, len-2);

    std::set<FileBlockDesc> newBlocks;

    assert(blocks.allInFileBlocks(desc, newBlocks));

    assert(newBlocks.size() == exprReadSegNum);

    if(exprReadSegNum == 1)
    {
        auto newdesc = newBlocks.begin();
        assert(newdesc != newBlocks.end());
        assert(newBlocks.size() == 1);
        ELOG_DEBUG(newdesc->toString().c_str());
        ELOG_DEBUG(desc.toString().c_str());
        assert(newdesc->inSameLogicalRange(desc));
    }

    ELOG_INFO("  ->caseDiffStartSameEnd %s", green("OK").c_str());
}

//单读
void caseAfterWriteOneRead(ActiveFileBlocks &blocks, unsigned long off, unsigned long len , std::list<unsigned long> &logicalOff)
{
    ELOG_INFO(" ->caseAfterWriteOneRead %s",cyan("start").c_str());

    caseSameWriteRange(blocks, off, len , logicalOff.size());

    caseSameStartDiffEnd(blocks, off, len,logicalOff.size());

    caseDiffStartSameEnd(blocks, off, len,logicalOff.size());

    caseDiffStartDiffEnd(blocks, off, len,logicalOff.size());

    ELOG_INFO(" ->caseAfterWriteOneRead  %s", green("OK").c_str());
}

void assertSame(std::set<FileBlockDesc> &newBlock, FileBlockDesc &oldblock)
{
    auto newdesc = newBlock.begin();
    assert(newdesc != newBlock.end());
    assert(newBlock.size() == 1);
    ELOG_DEBUG(newdesc->toString().c_str());
    ELOG_DEBUG(oldblock.toString().c_str());
    assert(newdesc->inSameLogicalRange(oldblock));
}

void assertSameRange(std::set<FileBlockDesc> &newBlock, FileBlockDesc &oldblock, int exprReadSegNum)
{
    if(exprReadSegNum == 1)
    {
        assertSame(newBlock, oldblock);
    }
    assert(exprReadSegNum == newBlock.size());
    auto firstdesc = newBlock.begin();
    unsigned long logicalStart = firstdesc->dataOffInFile +firstdesc->file.off();
    unsigned long totallen = 0;
    for(auto block : newBlock)
    {
        totallen += block.dataSizeInFile;
    }
    assert(oldblock.dataOffInFile + oldblock.file.off() == logicalStart);
    assert(oldblock.dataSizeInFile == totallen);
}

void exprReadSegInTwoSeg(unsigned long firstPoint,unsigned long secondPoint,const FileBlockDesc &desc, int &exprReadSegNum)
{
    unsigned long start = desc.file.off() + desc.dataOffInFile;
    unsigned long end = start + desc.dataSizeInFile;
    assert(start >= firstPoint);
    if(secondPoint >= end)
    {//desc 在左边第一段范围
        exprReadSegNum = 1;
    }
    else if(secondPoint <= start)
    {//desc 在右边界外
        exprReadSegNum = 1;
    }
    else
    {
        exprReadSegNum = 2;
    }
}


void exprReadSegInThreeSeg(unsigned long firstPoint,unsigned long secondPoint, unsigned long thirdPoint,
                           const FileBlockDesc &desc, int &exprReadSegNum)
{
    unsigned long start = desc.file.off() + desc.dataOffInFile;
    unsigned long end = start + desc.dataSizeInFile;
    assert(start >= firstPoint);
    if(secondPoint >= end)
    {//desc 在左边第一段范围
        exprReadSegNum = 1;
    }
    else if(thirdPoint <= start)
    {//desc 在右边界外
        exprReadSegNum = 1;
    }
    else if(start >= secondPoint && end < thirdPoint)
    {//desc在最中间一段范围内
        exprReadSegNum = 1;
    }
    else if(start < secondPoint && end > thirdPoint)
    {
        exprReadSegNum = 3;
    }
    else
    {
        exprReadSegNum = 2;
    }
}

//根据两个区域判断数据的分段数量
void exprReadSegs(std::list<unsigned long> &logicalOff, FileBlockDesc &desc1 , FileBlockDesc &desc2 , int &exprReadSegNum1, int &exprReadSegNum2)
{
    if(logicalOff.size() == 1)
    {
        exprReadSegNum1 = 1;
        exprReadSegNum2 = 1;
        return;
    }
    auto logicalOffIt = logicalOff.begin();
    unsigned long FirstPoint = *logicalOffIt;
    ++logicalOffIt;
    unsigned long SecondPoint = *logicalOffIt;
    if(logicalOff.size() == 2)
    {
        exprReadSegInTwoSeg(FirstPoint, SecondPoint, desc1, exprReadSegNum1);
        exprReadSegInTwoSeg(FirstPoint, SecondPoint, desc2, exprReadSegNum2);
        return;
    }
    assert(logicalOff.size() == 3);
    ++logicalOffIt;
    unsigned long ThirdPoint = *logicalOffIt;
    exprReadSegInThreeSeg(FirstPoint, SecondPoint,ThirdPoint, desc1, exprReadSegNum1);
    exprReadSegInThreeSeg(FirstPoint, SecondPoint,ThirdPoint, desc2, exprReadSegNum2);
}

//两个不同读覆盖了写
void caseDiffCoverWrite(ActiveFileBlocks &blocks, unsigned long off, unsigned long len, std::list<unsigned long> &logicalOff) {
    ELOG_INFO("  ->caseDiffCoverWrite %s", cyan("start").c_str());
    unsigned long len1 = rand() % (len - 2) + 1;
    FileBlockDesc desc1("", off, len1);
    FileBlockDesc desc2("", off + len1, len - len1);

    ELOG_WARNING("\ndesc1:%s\n"
                  "des2:%s\n"
                  "block%s\n",
                  desc1.toString().c_str(),desc2.toString().c_str(), blocks.toString().c_str());

    std::set<FileBlockDesc> newBlocks1;
    std::set<FileBlockDesc> newBlocks2;

    int exprReadSegNum1, exprReadSegNum2;
    exprReadSegs(logicalOff, desc1, desc2, exprReadSegNum1, exprReadSegNum2);
    for (auto lOff: logicalOff)
    {
        ELOG_DEBUG("lOff:%lu", lOff);
    }
    ELOG_DEBUG("\ndesc1:%s seg1 = %d exprReadSegNum1 = %d\n"
                 "desc2:%s seg2 = %d exprReadSegNum2 = %d\n"
                 "blocks:%s\n"
    ,desc1.toString().c_str(), newBlocks1.size(),exprReadSegNum1
    ,desc2.toString().c_str(), newBlocks2.size(),exprReadSegNum2
    ,blocks.toString().c_str());

    assert(blocks.allInFileBlocks(desc1, newBlocks1));

    assert(blocks.allInFileBlocks(desc2, newBlocks2));

    assertSameRange(newBlocks1, desc1, exprReadSegNum1);
    assertSameRange(newBlocks2, desc2, exprReadSegNum2);

    ELOG_INFO("  ->caseDiffCoverWrite %s", green("OK").c_str());
}

void caseDiffReadNotConflict(ActiveFileBlocks &blocks, unsigned long off, unsigned long len,std::list<unsigned long> &logicalOff)
{
    ELOG_INFO("  ->caseDiffReadNotConflict %s",cyan("start").c_str());
    unsigned long len1 =  rand()%(len/2) + 1;
    FileBlockDesc desc1("",off+1,len1);
    FileBlockDesc desc2("",off + len1 + 1, len-len1 -2);

    std::set<FileBlockDesc> newBlocks1;
    std::set<FileBlockDesc> newBlocks2;

    assert(blocks.allInFileBlocks(desc1, newBlocks1));
    assert(blocks.allInFileBlocks(desc2, newBlocks2));

    int exprReadSegNum1, exprReadSegNum2;
    exprReadSegs(logicalOff, desc1, desc2, exprReadSegNum1, exprReadSegNum2);
    assertSameRange(newBlocks1, desc1, exprReadSegNum1);
    assertSameRange(newBlocks2, desc2, exprReadSegNum2);

    ELOG_INFO("  ->caseDiffReadNotConflict %s", green("OK").c_str());
}

void caseDiffReadConflict(ActiveFileBlocks &blocks, unsigned long off, unsigned long len, std::list<unsigned long> &logicalOff)
{
    ELOG_INFO("  ->caseDiffReadConflict %s",cyan("start").c_str());
    unsigned long len1 =  rand()%(len/2) + 1;
    FileBlockDesc desc1("",off+1,len1);
    FileBlockDesc desc2("",off + len1 - 1, len - len1 - len/4 );
    ELOG_WARNING("\ndesc1:%s\n"
               "des2:%s\n"
               "block%s\n",
               desc1.toString().c_str(),desc2.toString().c_str(), blocks.toString().c_str());

    std::set<FileBlockDesc> newBlocks1;
    std::set<FileBlockDesc> newBlocks2;

    assert(blocks.allInFileBlocks(desc1, newBlocks1));
    assert(blocks.allInFileBlocks(desc2, newBlocks2));

    int exprReadSegNum1, exprReadSegNum2;
    exprReadSegs(logicalOff, desc1, desc2, exprReadSegNum1, exprReadSegNum2);
    assertSameRange(newBlocks1, desc1, exprReadSegNum1);
    assertSameRange(newBlocks2, desc2, exprReadSegNum2);

    ELOG_INFO("  ->caseDiffReadNotConflict %s", green("OK").c_str());
}

//两次读
void caseAfterWriteTwoRead(ActiveFileBlocks &blocks, unsigned long off, unsigned long len, std::list<unsigned long> &logicalOff)
{
    ELOG_INFO(" ->caseAfterWriteTwoRead %s",cyan("start").c_str());

    caseDiffCoverWrite(blocks, off, len ,logicalOff);

    caseDiffReadNotConflict(blocks, off, len , logicalOff);

    caseDiffReadConflict(blocks, off, len , logicalOff);

    ELOG_INFO(" ->caseAfterWriteTwoRead  %s", green("OK").c_str());
}

//一个完整写完整度
void caseOneWrite()
{
    ELOG_INFO("caseOneWrite %s",cyan("start").c_str());
    ActiveFileBlocks blocks;
    std::set<FileBlockDesc> fileBlocklist;
    std::set<FileDesc> ToRemovefiles;
    FileBlockDesc desc("./test_3_100",3, 100);
    fileBlocklist.insert(desc);

    blocks.CoverFiles(fileBlocklist, ToRemovefiles);

    assert(ToRemovefiles.empty());

    std::list<unsigned long> logicalOff;
    logicalOff.push_back(desc.file.off());

    caseAfterWriteOneRead(blocks, desc.file.off(), desc.dataSizeInFile, logicalOff);

    caseAfterWriteTwoRead(blocks, desc.file.off(), desc.dataSizeInFile, logicalOff);

    ELOG_INFO("caseOneWrite %s", green("OK").c_str());
}

//写范围不交叉，不紧邻
void caseTwoWriteNotConflictFar()
{
    ELOG_INFO("+caseTwoWriteNotConflictFar %s",cyan("start").c_str());
    ActiveFileBlocks blocks;
    std::set<FileBlockDesc> fileBlocklist1, fileBlocklist2;
    std::set<FileDesc> ToRemovefiles;
    FileBlockDesc desc1("./test_3_100",3, 100);
    fileBlocklist1.insert(desc1);

    FileBlockDesc desc2("./test_105_100",105, 100);
    fileBlocklist2.insert(desc2);

    blocks.CoverFiles(fileBlocklist1, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist2, ToRemovefiles);
    assert(ToRemovefiles.empty());

    std::list<unsigned long> logicalOff1;
    logicalOff1.push_back(desc1.file.off());

    std::list<unsigned long> logicalOff2;
    logicalOff2.push_back(desc2.file.off());

    caseAfterWriteOneRead(blocks, desc1.file.off(), desc1.dataSizeInFile, logicalOff1);

    caseAfterWriteOneRead(blocks, desc2.file.off(), desc2.dataSizeInFile, logicalOff2);

    caseAfterWriteTwoRead(blocks, desc1.file.off(), desc1.dataSizeInFile, logicalOff1);

    caseAfterWriteTwoRead(blocks, desc2.file.off(), desc2.dataSizeInFile, logicalOff2);

    ELOG_INFO("+caseTwoWriteNotConflictFar %s", green("OK").c_str());
}

//写范围不交叉，但紧邻
void caseTwoWriteConflictNear()
{
    ELOG_INFO("+caseTwoWriteConflictNear %s",cyan("start").c_str());
    ActiveFileBlocks blocks;
    std::set<FileBlockDesc> fileBlocklist1, fileBlocklist2;
    std::set<FileDesc> ToRemovefiles;
    FileBlockDesc desc1("./test_3_100",3, 100);
    fileBlocklist1.insert(desc1);

    FileBlockDesc desc2("./test_103_100",103, 100);
    fileBlocklist2.insert(desc2);

    blocks.CoverFiles(fileBlocklist1, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist2, ToRemovefiles);
    assert(ToRemovefiles.empty());

    std::list<unsigned long> logicalOff;
    logicalOff.push_back(desc1.file.off());
    logicalOff.push_back(desc2.file.off());

    caseAfterWriteOneRead(blocks, 3, 200, logicalOff);

    caseAfterWriteTwoRead(blocks, 3, 200, logicalOff);

    ELOG_INFO("+caseTwoWriteConflictNear %s", green("OK").c_str());
}

//两次写第一次覆盖第二次
void caseTwoWriteFirstCoverSecond()
{
    ELOG_INFO("+caseTwoWriteFirstCoverSecond %s",cyan("start").c_str());
    ActiveFileBlocks blocks;
    std::set<FileBlockDesc> fileBlocklist1, fileBlocklist2;
    std::set<FileDesc> ToRemovefiles;
    FileBlockDesc desc1("./test_5_100",5, 100);
    fileBlocklist1.insert(desc1);

    FileBlockDesc desc2("./test_20_60",20, 60);
    fileBlocklist2.insert(desc2);

    blocks.CoverFiles(fileBlocklist1, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist2, ToRemovefiles);
    assert(ToRemovefiles.empty());

    std::list<unsigned long> logicalOff;
    logicalOff.push_back(desc1.file.off());
    logicalOff.push_back(desc2.file.off());
    logicalOff.push_back(desc2.file.off() + desc2.dataSizeInFile);

    caseAfterWriteOneRead(blocks, 5, 100, logicalOff);

    caseAfterWriteTwoRead(blocks, 5, 100, logicalOff);

    ELOG_INFO("+caseTwoWriteFirstCoverSecond %s", green("OK").c_str());
}

//两次写第二次覆盖第一次
void caseTwoWriteSecondCoverFirst()
{
    ELOG_INFO("+caseTwoWSecondCoverFirst %s",cyan("start").c_str());
    ActiveFileBlocks blocks;
    std::set<FileBlockDesc> fileBlocklist1, fileBlocklist2;
    std::set<FileDesc> ToRemovefiles;
    FileBlockDesc desc1("./test_20_60",20, 60);
    fileBlocklist1.insert(desc1);

    FileBlockDesc desc2("./test_5_100",5, 100);
    fileBlocklist2.insert(desc2);

    blocks.CoverFiles(fileBlocklist1, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist2, ToRemovefiles);
    assert(ToRemovefiles.size() == 1);
    assert(*ToRemovefiles.begin() == FileDesc("./test_20_60",20, 60));

    std::list<unsigned long> logicalOff;
    logicalOff.push_back(desc2.file.off());

    caseAfterWriteOneRead(blocks, 5, 100, logicalOff);

    caseAfterWriteTwoRead(blocks, 5, 100, logicalOff);

    ELOG_INFO("+caseTwoWSecondCoverFirst %s", green("OK").c_str());
}


//两次写两次写完全相同
void caseTwoWriteSameRange()
{
    ELOG_INFO("+caseTwoWriteSameRange %s",cyan("start").c_str());
    ActiveFileBlocks blocks;
    std::set<FileBlockDesc> fileBlocklist1, fileBlocklist2;
    std::set<FileDesc> ToRemovefiles;
    FileBlockDesc desc1("./test_20_80",20, 80);
    fileBlocklist1.insert(desc1);

    FileBlockDesc desc2("./test_20_80",20, 80);
    fileBlocklist2.insert(desc2);

    blocks.CoverFiles(fileBlocklist1, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist2, ToRemovefiles);
    assert(ToRemovefiles.empty());

    std::list<unsigned long> logicalOff;
    logicalOff.push_back(desc1.file.off());

    caseAfterWriteOneRead(blocks, 20, 80, logicalOff);

    caseAfterWriteTwoRead(blocks, 20, 80, logicalOff);

    ELOG_INFO("+caseTwoWriteSameRange %s", green("OK").c_str());
}

//两段写
void caseTwoWrite()
{
    ELOG_INFO("caseTwoWrite %s",cyan("start").c_str());

    caseTwoWriteNotConflictFar();

    caseTwoWriteConflictNear();

    caseTwoWriteFirstCoverSecond();

    caseTwoWriteSecondCoverFirst();

    caseTwoWriteSameRange();

    ELOG_INFO("caseTwoWrite %s", green("OK").c_str());
}


//三次写第一次覆盖第二次、第三次
void caseThreeWriteFirstCoverAfter()
{
    ELOG_INFO("+caseThreeWriteFirstCoverAfter %s",cyan("start").c_str());
    ActiveFileBlocks blocks;
    std::set<FileBlockDesc> fileBlocklist1, fileBlocklist2, fileBlocklist3;
    std::set<FileDesc> ToRemovefiles;
    FileBlockDesc desc1("./test_8_60",8, 60);
    fileBlocklist1.insert(desc1);

    FileBlockDesc desc2("./test_8_30",8, 30);
    fileBlocklist2.insert(desc2);

    FileBlockDesc desc3("./test_38_30",38, 30);
    fileBlocklist3.insert(desc3);

    std::list<unsigned long> logicalOff;
    logicalOff.push_back(desc2.file.off());
    logicalOff.push_back(desc3.file.off());

    blocks.CoverFiles(fileBlocklist1, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist2, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist3, ToRemovefiles);

    for (auto lOff: logicalOff)
    {
        ELOG_DEBUG("lOff:%lu", lOff);
    }
    ELOG_WARNING("\ndesc1:%s\n"
               "desc2:%s \n"
               "desc3:%s \n"
               "blocks:%s\n"
                ,desc1.toString().c_str(),
                desc2.toString().c_str(),
                desc3.toString().c_str(),
                blocks.toString().c_str());

    assert(ToRemovefiles.size() == 1);
    assert(*ToRemovefiles.begin() == FileDesc("./test_8_60",8, 60));


    caseAfterWriteOneRead(blocks, 8, 60, logicalOff);

    ELOG_WARNING("blocks:%s\n"
                 ,blocks.toString().c_str());
    caseAfterWriteTwoRead(blocks, 8, 60, logicalOff);

    ELOG_INFO("+caseThreeWriteFirstCoverAfter %s", green("OK").c_str());
}



//三次写第一次覆盖第二次、第三次
void caseThreeWriteFirstCoverAfter2()
{
    ELOG_INFO("+caseThreeWriteFirstCoverAfter2 %s",cyan("start").c_str());
    ActiveFileBlocks blocks;
    std::set<FileBlockDesc> fileBlocklist1, fileBlocklist2, fileBlocklist3;
    std::set<FileDesc> ToRemovefiles;
    FileBlockDesc desc1("./test_8_60",8, 60);
    fileBlocklist1.insert(desc1);

    FileBlockDesc desc2("./test_38_30",38, 30);
    fileBlocklist2.insert(desc2);

    FileBlockDesc desc3("./test_8_30",8, 30);
    fileBlocklist3.insert(desc3);

    std::list<unsigned long> logicalOff;
    logicalOff.push_back(desc3.file.off());
    logicalOff.push_back(desc2.file.off());

    blocks.CoverFiles(fileBlocklist1, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist2, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist3, ToRemovefiles);

    for (auto lOff: logicalOff)
    {
        ELOG_DEBUG("lOff:%lu", lOff);
    }
    ELOG_WARNING("\ndesc1:%s\n"
                 "desc2:%s \n"
                 "desc3:%s \n"
                 "blocks:%s\n"
    ,desc1.toString().c_str(),
                 desc2.toString().c_str(),
                 desc3.toString().c_str(),
                 blocks.toString().c_str());

    assert(ToRemovefiles.size() == 1);
    assert(*ToRemovefiles.begin() == FileDesc("./test_8_60",8, 60));


    caseAfterWriteOneRead(blocks, 8, 60, logicalOff);

    ELOG_WARNING("blocks:%s\n"
    ,blocks.toString().c_str());
    caseAfterWriteTwoRead(blocks, 8, 60, logicalOff);

    ELOG_INFO("+caseThreeWriteFirstCoverAfter2 %s", green("OK").c_str());
}


//三次写第二次覆盖其他
void caseThreeWriteSecodCoverOth()
{
    ELOG_INFO("+caseThreeWriteSecodCoverOth %s",cyan("start").c_str());
    ActiveFileBlocks blocks;
    std::set<FileBlockDesc> fileBlocklist1, fileBlocklist2, fileBlocklist3;
    std::set<FileDesc> ToRemovefiles;
    FileBlockDesc desc1("./test_35_60",35, 60);
    fileBlocklist1.insert(desc1);

    FileBlockDesc desc2("./test_5_90",5, 90);
    fileBlocklist2.insert(desc2);

    FileBlockDesc desc3("./test_5_30",5, 30);
    fileBlocklist3.insert(desc3);

    std::list<unsigned long> logicalOff;
    logicalOff.push_back(desc3.file.off());
    logicalOff.push_back(desc1.file.off());

    blocks.CoverFiles(fileBlocklist1, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist2, ToRemovefiles);
    assert(ToRemovefiles.size() == 1);
    assert(*ToRemovefiles.begin() == FileDesc(desc1.file.fileName(),desc1.file.off(), desc1.dataSizeInFile));
    ToRemovefiles.clear();
    blocks.CoverFiles(fileBlocklist3, ToRemovefiles);
    assert(ToRemovefiles.empty());
    for (auto lOff: logicalOff)
    {
        ELOG_DEBUG("lOff:%lu", lOff);
    }
    ELOG_WARNING("\ndesc1:%s\n"
                 "desc2:%s \n"
                 "desc3:%s \n"
                 "blocks:%s\n"
    ,desc1.toString().c_str(),
                 desc2.toString().c_str(),
                 desc3.toString().c_str(),
                 blocks.toString().c_str());

    caseAfterWriteOneRead(blocks, 5, 90, logicalOff);

    ELOG_WARNING("blocks:%s\n"
    ,blocks.toString().c_str());
    caseAfterWriteTwoRead(blocks, 5, 90, logicalOff);

    ELOG_INFO("+caseThreeWriteFirstCoverAfter %s", green("OK").c_str());
}


//三次写第三次覆盖第其他
void caseThreeWriteThirdCoverOth()
{
    ELOG_INFO("+caseThreeWriteThirdCoverOth %s",cyan("start").c_str());
    ActiveFileBlocks blocks;
    std::set<FileBlockDesc> fileBlocklist1, fileBlocklist2, fileBlocklist3;
    std::set<FileDesc> ToRemovefiles;
    FileBlockDesc desc1("./test_32_60",32, 60);
    fileBlocklist1.insert(desc1);

    FileBlockDesc desc2("./test_2_30",2, 30);
    fileBlocklist2.insert(desc2);

    FileBlockDesc desc3("./test_2_90",2, 90);
    fileBlocklist3.insert(desc3);

    std::list<unsigned long> logicalOff;
    logicalOff.push_back(desc3.file.off());

    blocks.CoverFiles(fileBlocklist1, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist2, ToRemovefiles);
    assert(ToRemovefiles.empty());

    blocks.CoverFiles(fileBlocklist3, ToRemovefiles);

    assert(ToRemovefiles.size() == 2);
    auto  it = ToRemovefiles.begin();
    assert(*it == FileDesc(desc2.file.fileName(),desc2.file.off(), desc2.dataSizeInFile));
    ++it;
    assert(*it == FileDesc(desc1.file.fileName(),desc1.file.off(), desc1.dataSizeInFile));

    for (auto lOff: logicalOff)
    {
        ELOG_DEBUG("lOff:%lu", lOff);
    }
    ELOG_WARNING("\ndesc1:%s\n"
                 "desc2:%s \n"
                 "desc3:%s \n"
                 "blocks:%s\n"
    ,desc1.toString().c_str(),
                 desc2.toString().c_str(),
                 desc3.toString().c_str(),
                 blocks.toString().c_str());

    caseAfterWriteOneRead(blocks, 2, 90, logicalOff);

    ELOG_WARNING("blocks:%s\n"
    ,blocks.toString().c_str());
    caseAfterWriteTwoRead(blocks, 2, 90, logicalOff);

    ELOG_INFO("+caseThreeWriteThirdCoverOth %s", green("OK").c_str());
}

//三次写互不交叉
void caseThreeWriteNotConflict()
{
    ELOG_INFO("+caseThreeWriteThirdCoverOth %s",cyan("start").c_str());
    ActiveFileBlocks blocks;
    std::set<FileBlockDesc> fileBlocklist1, fileBlocklist2, fileBlocklist3;
    std::set<FileDesc> ToRemovefiles;
    FileBlockDesc desc1("./test_12_35",12, 35);
    fileBlocklist1.insert(desc1);

    FileBlockDesc desc2("./test_47_22",47, 22);
    fileBlocklist2.insert(desc2);

    FileBlockDesc desc3("./test_69_14",69, 14);
    fileBlocklist3.insert(desc3);

    std::list<unsigned long> logicalOff;
    logicalOff.push_back(desc1.file.off());
    logicalOff.push_back(desc2.file.off());
    logicalOff.push_back(desc3.file.off());


    blocks.CoverFiles(fileBlocklist1, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist2, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist3, ToRemovefiles);
    assert(ToRemovefiles.empty());

    for (auto lOff: logicalOff)
    {
        ELOG_DEBUG("lOff:%lu", lOff);
    }
    ELOG_WARNING("\ndesc1:%s\n"
                 "desc2:%s \n"
                 "desc3:%s \n"
                 "blocks:%s\n"
    ,desc1.toString().c_str(),
                 desc2.toString().c_str(),
                 desc3.toString().c_str(),
                 blocks.toString().c_str());

    caseAfterWriteOneRead(blocks, 12, 71, logicalOff);

    ELOG_WARNING("blocks:%s\n"
    ,blocks.toString().c_str());
    caseAfterWriteTwoRead(blocks, 12, 71, logicalOff);

    ELOG_INFO("+caseThreeWriteThirdCoverOth %s", green("OK").c_str());
}


//三次写交叉但是不覆盖
void caseThreeWriteConflictNoCover()
{
    ELOG_INFO("+caseThreeWConflictNoCover %s",cyan("start").c_str());
    ActiveFileBlocks blocks;
    std::set<FileBlockDesc> fileBlocklist1, fileBlocklist2, fileBlocklist3;
    std::set<FileDesc> ToRemovefiles;
    FileBlockDesc desc1("./test_12_35",12, 35);
    fileBlocklist1.insert(desc1);

    FileBlockDesc desc2("./test_45_25",45, 25);
    fileBlocklist2.insert(desc2);

    FileBlockDesc desc3("./test_69_14",69, 14);
    fileBlocklist3.insert(desc3);

    std::list<unsigned long> logicalOff;
    logicalOff.push_back(desc1.file.off());
    logicalOff.push_back(desc2.file.off());
    logicalOff.push_back(desc3.file.off());


    blocks.CoverFiles(fileBlocklist1, ToRemovefiles);
    assert(ToRemovefiles.empty());
    ELOG_WARNING("\ndesc1:%s\n"
                 "desc2:%s \n"
                 "desc3:%s \n"
                 "blocks:%s\n"
    ,desc1.toString().c_str(),
                 desc2.toString().c_str(),
                 desc3.toString().c_str(),
                 blocks.toString().c_str());

    blocks.CoverFiles(fileBlocklist2, ToRemovefiles);
    assert(ToRemovefiles.empty());
    ELOG_WARNING("\ndesc1:%s\n"
                 "desc2:%s \n"
                 "desc3:%s \n"
                 "blocks:%s\n"
    ,desc1.toString().c_str(),
                 desc2.toString().c_str(),
                 desc3.toString().c_str(),
                 blocks.toString().c_str());
    blocks.CoverFiles(fileBlocklist3, ToRemovefiles);
    assert(ToRemovefiles.empty());

    for (auto lOff: logicalOff)
    {
        ELOG_DEBUG("lOff:%lu", lOff);
    }
    ELOG_WARNING("\ndesc1:%s\n"
                 "desc2:%s \n"
                 "desc3:%s \n"
                 "blocks:%s\n"
    ,desc1.toString().c_str(),
                 desc2.toString().c_str(),
                 desc3.toString().c_str(),
                 blocks.toString().c_str());

    caseAfterWriteOneRead(blocks, 12, 71, logicalOff);

    ELOG_WARNING("blocks:%s\n"
    ,blocks.toString().c_str());
    caseAfterWriteTwoRead(blocks, 12, 71, logicalOff);

    ELOG_INFO("+caseThreeWConflictNoCover %s", green("OK").c_str());
}


//三次写交叉 一与二交叉，二与三交叉，但一与三不交叉 3全部是1和2的子集
void caseThreeWriteConflictCotainM1()
{
    ELOG_INFO("+caseThreeWriteConflictCotainM1 %s",cyan("start").c_str());
    ActiveFileBlocks blocks;
    std::set<FileBlockDesc> fileBlocklist1, fileBlocklist2, fileBlocklist3;
    std::set<FileDesc> ToRemovefiles;


    FileBlockDesc desc1("./test_12_50",12, 50);
    fileBlocklist1.insert(desc1);

    FileBlockDesc desc2("./test_47_22",47, 22);
    fileBlocklist2.insert(desc2);

    FileBlockDesc desc3("./test_62_21",62, 21);
    fileBlocklist3.insert(desc3);

    std::list<unsigned long> logicalOff;
    logicalOff.push_back(desc1.file.off());
    logicalOff.push_back(desc2.file.off());
    logicalOff.push_back(desc3.file.off());


    blocks.CoverFiles(fileBlocklist1, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist2, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist3, ToRemovefiles);
    assert(ToRemovefiles.empty());

    for (auto lOff: logicalOff)
    {
        ELOG_DEBUG("lOff:%lu", lOff);
    }
    ELOG_WARNING("\ndesc1:%s\n"
                 "desc2:%s \n"
                 "desc3:%s \n"
                 "blocks:%s\n"
    ,desc1.toString().c_str(),
                 desc2.toString().c_str(),
                 desc3.toString().c_str(),
                 blocks.toString().c_str());

    caseAfterWriteOneRead(blocks, 12, 71, logicalOff);

    ELOG_WARNING("blocks:%s\n"
    ,blocks.toString().c_str());
    caseAfterWriteTwoRead(blocks, 12, 71, logicalOff);

    ELOG_INFO("+caseThreeWriteConflictCotainM1 %s", green("OK").c_str());
}


//三次写交叉 一与二交叉，二与三交叉，但一与三不交叉 3全部是1和2的子集
void caseThreeWriteConflictCotainM2()
{
    ELOG_INFO("+caseThreeWriteConflictCotainM1 %s",cyan("start").c_str());
    ActiveFileBlocks blocks;
    std::set<FileBlockDesc> fileBlocklist1, fileBlocklist2, fileBlocklist3;
    std::set<FileDesc> ToRemovefiles;

    FileBlockDesc desc1("./test_47_22",47, 22);
    fileBlocklist1.insert(desc1);

    FileBlockDesc desc2("./test_12_50",12, 50);
    fileBlocklist2.insert(desc2);


    FileBlockDesc desc3("./test_62_21",62, 21);
    fileBlocklist3.insert(desc3);

    std::list<unsigned long> logicalOff;
    logicalOff.push_back(desc2.file.off());
    logicalOff.push_back(desc3.file.off());


    blocks.CoverFiles(fileBlocklist1, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist2, ToRemovefiles);
    assert(ToRemovefiles.empty());
    blocks.CoverFiles(fileBlocklist3, ToRemovefiles);
    assert(ToRemovefiles.size() == 1);
    assert(*ToRemovefiles.begin() == FileDesc(desc1.file.fileName(),desc1.file.off(), desc1.dataSizeInFile));

    for (auto lOff: logicalOff)
    {
        ELOG_DEBUG("lOff:%lu", lOff);
    }
    ELOG_WARNING("\ndesc1:%s\n"
                 "desc2:%s \n"
                 "desc3:%s \n"
                 "blocks:%s\n"
    ,desc1.toString().c_str(),
                 desc2.toString().c_str(),
                 desc3.toString().c_str(),
                 blocks.toString().c_str());

    caseAfterWriteOneRead(blocks, 12, 71, logicalOff);

    ELOG_WARNING("blocks:%s\n"
    ,blocks.toString().c_str());
    caseAfterWriteTwoRead(blocks, 12, 71, logicalOff);

    ELOG_INFO("+caseThreeWriteConflictCotainM1 %s", green("OK").c_str());
}


void caseThreeWrite()
{
    ELOG_INFO("caseThreeWrite %s",cyan("start").c_str());

    caseThreeWriteFirstCoverAfter();

    caseThreeWriteFirstCoverAfter2();

    caseThreeWriteSecodCoverOth();

    caseThreeWriteThirdCoverOth();

    caseThreeWriteNotConflict();

    caseThreeWriteConflictNoCover();

    caseThreeWriteConflictCotainM1();

    caseThreeWriteConflictCotainM2();

    ELOG_INFO("caseThreeWrite %s", green("OK").c_str());
}

int main(int argc, const char * argv[])
{
    initColor();

    caseOneWrite();

    caseTwoWrite();

    caseThreeWrite();

    ELOG_INFO("+++++++All cases are %s", green("OK").c_str());

    return 0;
}