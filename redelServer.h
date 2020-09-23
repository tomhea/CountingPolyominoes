#ifndef COUNTINGPOLYOMINOES_REDELSERVER_H
#define COUNTINGPOLYOMINOES_REDELSERVER_H

#include "defs.h"


bool canIFinishIt(const char* graphFilePath, u32 steps, u64* result);
int mainServer(const char* graphFilePath, u32 steps, int approxNumOfJobs, const char* jobBasePath);

#endif //COUNTINGPOLYOMINOES_REDELSERVER_H
