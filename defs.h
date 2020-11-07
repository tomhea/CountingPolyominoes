#ifndef COUNTINGPOLYOMINOES_DEFS_H
#define COUNTINGPOLYOMINOES_DEFS_H

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <string>
#include <unistd.h>

typedef unsigned int u32;
typedef unsigned long long u64;

using namespace std;


#define TEMP_FILE_SUFFIX "_temp"

#define MAX_PATH_LEN 256

#define BACKUP_INTERVALS  1000000000     // ~every 5 seconds
#define COUNTER_THRESHOLD 2000000000     // Timeout after ~10-20 seconds

// Polyiamonds       // https://oeis.org/A001420/b001420.txt
// Polyominoes       // https://oeis.org/A001168/b001168.txt
// Polycubes         // https://oeis.org/A001931/b001931.txt


// TODO: change to cpp initializations in all file
// TODO: add functions commentary

#endif //COUNTINGPOLYOMINOES_DEFS_H
