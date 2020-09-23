#include "backups.h"


inline void recover(bool mainBackup, const char* backUpPath, const char* tempBackUpPath, int& isNewJob,
                    u32& initSteps, u32& steps, u64& counted, u64& nextBackup, u32& graphSize, bool* nodesFound,
                    int* startOfUntriedSet, int*& untriedSetEndPtr,
                    int** startOfOldUntriedSetEndStack, int**& oldUntriedSetEndStackPtr,
                    int** startOfUntriedSetStack, int**& untriedSetStackPtr) {
    FILE *f;
    f = fopen(mainBackup ? backUpPath : tempBackUpPath, "rb");
    int temp; u64 temp64;

    fread(&temp, sizeof(u32), 1, f);
    initSteps = temp;
    fread(&temp, sizeof(u32), 1, f);
    steps = temp;
    fread(&temp64, sizeof(u64), 1, f);
    counted = temp64;
    fread(&temp, sizeof(int), 1, f);
    isNewJob = temp;
    fread(&temp64, sizeof(u64), 1, f);
    nextBackup = temp64;

    fread(&temp, sizeof(u32), 1, f);
    graphSize = temp;
    fread(nodesFound, sizeof(bool), graphSize, f);

    int untriedSetEndIdx;
    fread(&untriedSetEndIdx, sizeof(int), 1, f);
    fread(startOfUntriedSet, sizeof(int), untriedSetEndIdx, f);
    untriedSetEndPtr = startOfUntriedSet + untriedSetEndIdx;

    int* prevStartOfUntriedSet;
    fread(&prevStartOfUntriedSet, sizeof(int*), 1, f);

    int oldUntriedSetEndStackIdx;
    fread(&oldUntriedSetEndStackIdx, sizeof(int), 1, f);
    fread(startOfOldUntriedSetEndStack, sizeof(int*), oldUntriedSetEndStackIdx, f);
    oldUntriedSetEndStackPtr = startOfOldUntriedSetEndStack + oldUntriedSetEndStackIdx;
    for (int i = 0; i < oldUntriedSetEndStackIdx; i++)
        startOfOldUntriedSetEndStack[i] = startOfUntriedSet + (startOfOldUntriedSetEndStack[i] - prevStartOfUntriedSet);

    int untriedSetStackIdx;
    fread(&untriedSetStackIdx, sizeof(int), 1, f);
    fread(startOfUntriedSetStack, sizeof(int*), untriedSetStackIdx, f);
    untriedSetStackPtr = startOfUntriedSetStack + untriedSetStackIdx;
    for (int i = 0; i < untriedSetStackIdx; i++)
        startOfUntriedSetStack[i] = startOfUntriedSet + (startOfUntriedSetStack[i] - prevStartOfUntriedSet);

    fclose(f);
    if (!mainBackup)
        rename(tempBackUpPath, backUpPath);     // finish main backup
}

inline void doBackup(const char* backUpPath, const char* tempBackUpPath, int isNewJob,
                     u32 initSteps, u32 steps, u64 counted, u64 nextBackup, u32 graphSize, bool* nodesFound,
                     int* startOfUntriedSet, int untriedSetEndIdx,
                     int** startOfOldUntriedSetEndStack, int oldUntriedSetEndStackIdx,
                     int** startOfUntriedSetStack, int untriedSetStackIdx) {
    FILE *f;
    f = fopen(tempBackUpPath , "wb");

    fwrite(&initSteps, sizeof(u32), 1, f);
    fwrite(&steps, sizeof(u32), 1, f);
    fwrite(&counted, sizeof(u64), 1, f);
    fwrite(&isNewJob, sizeof(int), 1, f);
    fwrite(&nextBackup, sizeof(u64), 1, f);

    fwrite(&graphSize, sizeof(u32), 1, f);
    fwrite(nodesFound, sizeof(bool), graphSize, f);

    fwrite(&untriedSetEndIdx, sizeof(int), 1, f);
    fwrite(startOfUntriedSet, sizeof(int), untriedSetEndIdx, f);

    fwrite(&startOfUntriedSet, sizeof(int*), 1, f);

    fwrite(&oldUntriedSetEndStackIdx, sizeof(int), 1, f);
    fwrite(startOfOldUntriedSetEndStack, sizeof(int*), oldUntriedSetEndStackIdx, f);

    fwrite(&untriedSetStackIdx, sizeof(int), 1, f);
    fwrite(startOfUntriedSetStack, sizeof(int*), untriedSetStackIdx, f);

    fclose(f);

    remove(backUpPath);
    rename(tempBackUpPath, backUpPath);
}
