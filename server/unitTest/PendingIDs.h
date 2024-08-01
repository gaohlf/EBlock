//
// Created by 20075 on 2024/7/4.
//

#ifndef EBLOCK_PENDINGIDS_H
#define EBLOCK_PENDINGIDS_H
#include <mutex>
#include <map>
#include <list>
#include <iostream>
#include <chrono>

class IdContext
{
public:
    int id;
    std::list<int> conflictIDS;

    IdContext() = default;

    explicit IdContext(int newID) :id(newID)
    {};

    IdContext(const IdContext & other) :id(other.id)
    {
        std::copy(other.conflictIDS.begin(), other.conflictIDS.end(), this->conflictIDS.begin());
    }

    IdContext & operator =(const IdContext & other)
    {
        id = other.id;
        std::copy(other.conflictIDS.begin(), other.conflictIDS.end(), this->conflictIDS.begin());
        return *this;
    }

    bool conflictWith(int newID)
    {
        for(auto it = conflictIDS.begin(); it != conflictIDS.end();++it)
        {
            if(newID == *it)
            {
                return true;
            }
        }
        return false;
    }

};

class PendingIDs {
private:
    std::mutex m;
    std::map<int, IdContext *> pendingIDTimes;
    std::map<int, IdContext> idTimes;
private:
    //记录当前context和当前pendingID中的所有 context产生了冲突
    void recordConflict(IdContext *newContext);
public:
    //记录ID 和 当前时间
    void recordIDNow(int id);
    //移除ID
    void removeIDNow(int id);

    bool pendingEmpty();

    //判断两两是否冲突
    bool doTheyExecInSameTime(int idA, int idB);
};


#endif //EBLOCK_PENDINGIDS_H
