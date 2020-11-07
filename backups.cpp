#include "backups.h"


void recover(bool mainBackup, const char* backUpPath, const char* tempBackUpPath, int& isNewJob,
             u32& initSteps, u32& steps, u64& counted, u64& nextBackup, u32& graphSize, bool* nodesFound,
             int* startOfUntriedSet, int*& untriedSetEndPtr,
             int** startOfOldUntriedSetEndStack, int**& oldUntriedSetEndStackPtr,
             int** startOfUntriedSetStack, int**& untriedSetStackPtr) {
    FILE *f;
    f = fopen(mainBackup ? backUpPath : tempBackUpPath, "rb");
    int temp; u32 temp32; u64 temp64;

    fread(&temp32, 4, 1, f);
    initSteps = temp32;
    fread(&temp32, 4, 1, f);
    steps = temp32;
    fread(&temp64, 8, 1, f);
    counted = temp64;
    fread(&temp, 4, 1, f);
    isNewJob = temp;
    fread(&temp64, 8, 1, f);
    nextBackup = temp64;

    fread(&temp32, 4, 1, f);
    graphSize = temp32;
    fread(nodesFound, 1, graphSize, f);

    int untriedSetEndIdx;
    fread(&untriedSetEndIdx, 4, 1, f);
    fread(startOfUntriedSet, 4, untriedSetEndIdx, f);
    untriedSetEndPtr = startOfUntriedSet + untriedSetEndIdx;

    int* prevStartOfUntriedSet;
    fread(&prevStartOfUntriedSet, 8, 1, f);

    int oldUntriedSetEndStackIdx;
    fread(&oldUntriedSetEndStackIdx, 4, 1, f);
    fread(startOfOldUntriedSetEndStack, 8, oldUntriedSetEndStackIdx, f);
    oldUntriedSetEndStackPtr = startOfOldUntriedSetEndStack + oldUntriedSetEndStackIdx;
    for (int i = 0; i < oldUntriedSetEndStackIdx; i++)
        startOfOldUntriedSetEndStack[i] = startOfUntriedSet + (startOfOldUntriedSetEndStack[i] - prevStartOfUntriedSet);

    int untriedSetStackIdx;
    fread(&untriedSetStackIdx, 4, 1, f);
    fread(startOfUntriedSetStack, 8, untriedSetStackIdx, f);
    untriedSetStackPtr = startOfUntriedSetStack + untriedSetStackIdx;
    for (int i = 0; i < untriedSetStackIdx; i++)
        startOfUntriedSetStack[i] = startOfUntriedSet + (startOfUntriedSetStack[i] - prevStartOfUntriedSet);

    fclose(f);
    if (!mainBackup)
        rename(tempBackUpPath, backUpPath);     // finish main backup
}

void doBackup(const char* backUpPath, const char* tempBackUpPath, int isNewJob,
              u32 initSteps, u32 steps, u64 counted, u64 nextBackup, u32 graphSize, bool* nodesFound,
              int* startOfUntriedSet, int untriedSetEndIdx,
              int** startOfOldUntriedSetEndStack, int oldUntriedSetEndStackIdx,
              int** startOfUntriedSetStack, int untriedSetStackIdx) {
    FILE *f;
    f = fopen(tempBackUpPath , "wb");

    fwrite(&initSteps, 4, 1, f);
    fwrite(&steps, 4, 1, f);
    fwrite(&counted, 8, 1, f);
    fwrite(&isNewJob, 4, 1, f);
    fwrite(&nextBackup, 8, 1, f);

    fwrite(&graphSize, 4, 1, f);
    fwrite(nodesFound, 1, graphSize, f);

    fwrite(&untriedSetEndIdx, 4, 1, f);
    fwrite(startOfUntriedSet, 4, untriedSetEndIdx, f);

    fwrite(&startOfUntriedSet, 8, 1, f);

    fwrite(&oldUntriedSetEndStackIdx, 4, 1, f);
    fwrite(startOfOldUntriedSetEndStack, 8, oldUntriedSetEndStackIdx, f);

    fwrite(&untriedSetStackIdx, 4, 1, f);
    fwrite(startOfUntriedSetStack, 8, untriedSetStackIdx, f);

    fclose(f);

    remove(backUpPath);
    rename(tempBackUpPath, backUpPath);
}
