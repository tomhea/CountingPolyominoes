#ifndef COUNTINGPOLYOMINOES_REDELSERVER_H
#define COUNTINGPOLYOMINOES_REDELSERVER_H

#include "graphCreator.h"


bool canIFinishIt(int** graph, int originCell, u32 steps, u32 numOfNodes, u64* result, double maxTime);
int mainServer(string graphFilePath, u32 steps, int approxNumOfJobs, string jobBaseName);


#endif //COUNTINGPOLYOMINOES_REDELSERVER_H
