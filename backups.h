#ifndef COUNTINGPOLYOMINOES_BACKUPS_H
#define COUNTINGPOLYOMINOES_BACKUPS_H

#include "defs.h"


inline void recover (bool mainBackup, const char* backUpPath, const char* tempBackUpPath, int& isNewJob,
                     u32& initSteps, u32& steps, u64& counted, u64& nextBackup, u32& graphSize, bool* nodesFound,
                     int* startOfUntriedSet, int*& untriedSetEndPtr,
                     int** startOfOldUntriedSetEndStack, int**& oldUntriedSetEndStackPtr,
                     int** startOfUntriedSetStack, int**& untriedSetStackPtr);

inline void doBackup(const char* backUpPath, const char* tempBackUpPath, int isNewJob,
                     u32 initSteps, u32 steps, u64 counted, u64 nextBackup, u32 graphSize, bool* nodesFound,
                     int* startOfUntriedSet, int untriedSetEndIdx,
                     int** startOfOldUntriedSetEndStack, int oldUntriedSetEndStackIdx,
                     int** startOfUntriedSetStack, int untriedSetStackIdx);


#endif //COUNTINGPOLYOMINOES_BACKUPS_H
