//
// Created by 20075 on 2024/7/4.
//

#include <cassert>
#include "PendingIDs.h"

void PendingIDs::recordIDNow(int id) {
    std::lock_guard<std::mutex> lck(m);
    IdContext context(id);
    idTimes.insert(std::make_pair(id, context));

    if(!pendingIDTimes.empty())
    {
        recordConflict(&idTimes[id]);
    }

    pendingIDTimes.insert(std::make_pair(id, &idTimes[id]));

}

void PendingIDs::removeIDNow(int id) {
    std::lock_guard<std::mutex> lck(m);
    pendingIDTimes.erase(id);
}

bool PendingIDs::pendingEmpty() {
    std::lock_guard<std::mutex> lck(m);
    return pendingIDTimes.empty();
}

bool PendingIDs::doTheyExecInSameTime(int idA, int idB) {
    auto & contextA = idTimes[idA];
    auto & contextB = idTimes[idB];
//    std::cout<< "contextA:" << contextA.id << " conflict id:"<< contextB.id<<std::endl;
    if(contextA.conflictWith(idB))
    {
        return true;
    }
    assert(contextA.conflictWith(idB) == contextB.conflictWith(idA));

    return false;
}

void PendingIDs::recordConflict(IdContext *newContext) {
    for(auto & pendingIDTime : pendingIDTimes)
    {;
        IdContext *oldContext = pendingIDTime.second;
//        std::cout<< "newid:" << newContext->id << " conflict id:"<< oldContext->id<<std::endl;
        newContext->conflictIDS.push_back(oldContext->id);
        oldContext->conflictIDS.push_back(newContext->id);
    }
}
