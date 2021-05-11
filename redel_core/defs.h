#ifndef COUNTINGPOLYOMINOES_DEFS_H
#define COUNTINGPOLYOMINOES_DEFS_H

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <string>
#include <unistd.h>
#include <inttypes.h>


typedef uint32_t u32;
typedef uint64_t u64;

using namespace std;

#define REGULAR_GRAPH 0
#define MIN_MAX_CONDITION_GRAPH 1

#define TEMP_FILE_SUFFIX "_temp"

#define MAX_PATH_LEN 256

#define BACKUP_INTERVALS  1000000000     // ~every 5 seconds
#define COUNTER_THRESHOLD 2000000000     // Timeout after ~10-20 seconds

// Polyiamonds       // https://oeis.org/A001420/b001420.txt
// Polyominoes       // https://oeis.org/A001168/b001168.txt
// Polycubes         // https://oeis.org/A001931/b001931.txt


struct node {
    u32 num_of_neighbours;
    u64* neighbours;
    u64* extra_data;
};

struct graph {
    int graph_protocol;
    u64* extra_graph_data;
    u64 size;
    node* nodes;
};



// TODO: change to cpp initializations in all file
// TODO: add functions commentary

#endif //COUNTINGPOLYOMINOES_DEFS_H
