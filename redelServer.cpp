#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include <cstring>
#include <string>
#include "redelServer.h"


using namespace std;

#define BACKUP_INTERVALS 1000000000     // ~every 5 seconds

//#define Polyiamonds       // https://oeis.org/A001420/b001420.txt
//#define Polyominoes       // https://oeis.org/A001168/b001168.txt
#define Polycubes         // https://oeis.org/A001931/b001931.txt

#define BACKUP true

#define NUM_OF_NEIGHBOURS neighbours[0]
#define COUNT_STEP_BELOW 0

#ifdef Polyiamonds
#undef NUM_OF_NEIGHBOURS
#define NUM_OF_NEIGHBOURS 3
#undef COUNT_STEP_BELOW
#define COUNT_STEP_BELOW 1
#endif

#ifdef Polyominoes
#undef NUM_OF_NEIGHBOURS
#define NUM_OF_NEIGHBOURS 4
#endif

#ifdef Polycubes
#undef NUM_OF_NEIGHBOURS
#define NUM_OF_NEIGHBOURS 6
#endif




//typedef struct _list {
//    int value;
//    void* next;
//} * List;

///
/// \param nodes - the graph itself.
/// \param steps - number of nodes to add to the sub-graph.
/// \param nodesFound - boolean array of whether a node is in the sub-graph / untriedSet, or isn't.
/// \param untriedSet - array of size 'untriedSize' containing all the reachable-but-not-used of the sub-graph.
/// \param untriedSize - number of elements in the untried set.
/// \return - number of sub-graphs
u64 recCounter(int** nodes, u32 steps, bool* nodesFound, int* untriedSet, int* untriedSetEnd) {
    int node = *(untriedSet++);   // pop node from untried set
    int* neighbours = nodes[node];
    int numOfNeighbours = neighbours[0];

    if (steps == 2) {   // recursion stop condition
        untriedSetEnd += numOfNeighbours;
        for (int i = 1; i <= numOfNeighbours; i++)
            untriedSetEnd -= nodesFound[neighbours[i]];
        return 1 + untriedSetEnd-untriedSet;
    }

    int* oldUntriedSetEnd = untriedSetEnd;
    for (int i = 1; i <= numOfNeighbours; i++) {    // add new neighbours to untried set and found set
        int newNode = neighbours[i];
        if (!nodesFound[newNode]) {
            *(untriedSetEnd++) = newNode;
            nodesFound[newNode] = true;
        }
    }

    u64 counted = 1;        // count current sub-graph.
    while(untriedSet != untriedSetEnd)
        counted += recCounter(nodes, steps-1, nodesFound, untriedSet++, untriedSetEnd);

    while(oldUntriedSetEnd != untriedSetEnd)    // remove all new neighbours from found set.
        nodesFound[*(--untriedSetEnd)] = false;

    return counted;
}


///
/// \param nodes - the graph itself.
/// \param p - size of max sub-graphs.
/// \param n - number of vertices in graph.
/// \param originCell - starting node for any sub-graph.
/// \return - number of sub-graphs of 'nodes' contains at most 'p' nodes, including 'originCell'.
u64 simpleCountSubGraphs(int** nodes, u32 p, u32 n, int originCell) {
    if (p <= 1) return p;

    bool* nodesFound = (bool*)calloc(n, sizeof(bool));
    int* untriedSet = (int*)malloc(n * sizeof(int));
    nodesFound[originCell] = true;
    untriedSet[0] = originCell;

    u64 count = recCounter(nodes, p, nodesFound, untriedSet, untriedSet+1);

    free(nodesFound);
    free(untriedSet);
    return count;
}


inline void recover(bool mainBackup, const char* backUpPath, const char* tempBackUpPath, int& isNewJob,
                    int& steps, u64& counted, u64& nextBackup, int& graphSize, bool* nodesFound,
                    int* startOfUntriedSet, int*& untriedSetEndPtr,
                    int** startOfOldUntriedSetEndStack, int**& oldUntriedSetEndStackPtr,
                    int** startOfUntriedSetStack, int**& untriedSetStackPtr) {
    FILE *f;
    f = fopen(mainBackup ? backUpPath : tempBackUpPath, "rb");
    int temp; u64 temp64;

    fread(&temp, sizeof(int), 1, f);
    steps = temp;
    fread(&temp64, sizeof(u64), 1, f);
    counted = temp64;
    fread(&temp, sizeof(int), 1, f);
    isNewJob = temp;
    fread(&temp64, sizeof(u64), 1, f);
    nextBackup = temp64;

    fread(&temp, sizeof(int), 1, f);
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
                     int steps, u64 counted, u64 nextBackup, int graphSize, bool* nodesFound,
                     int* startOfUntriedSet, int untriedSetEndIdx,
                     int** startOfOldUntriedSetEndStack, int oldUntriedSetEndStackIdx,
                     int** startOfUntriedSetStack, int untriedSetStackIdx) {
    FILE *f;
    f = fopen(tempBackUpPath , "wb");

    fwrite(&steps, sizeof(int), 1, f);
    fwrite(&counted, sizeof(u64), 1, f);
    fwrite(&isNewJob, sizeof(int), 1, f);
    fwrite(&nextBackup, sizeof(u64), 1, f);

    fwrite(&graphSize, sizeof(int), 1, f);
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

///
/// \param nodes - the graph itself.
/// \param steps - number of nodes to add to the sub-graph.
/// \param nodesFound - boolean array of whether a node is in the sub-graph / untriedSet, or isn't.
/// \param untriedSet - array of size 'untriedSize' containing all the reachable-but-not-used of the sub-graph.
/// \param untriedSize - number of elements in the untried set.
/// \return - number of sub-graphs
u64 recCounterGOTO(int** nodes, bool* nodesFound, int* untriedSet, int* untriedSetEnd, int** oldUntriedSetEndStack, int** untriedSetStack, int initSteps, int graphSize, const char* backUpPath, const char* tempBackUpPath) {
    //TODO declare register
    int *neighbours, *oldUntriedSetEnd;
    u64 counted = 0;
    u64 nextBackup = BACKUP_INTERVALS;
    int steps = initSteps;
    int isNewJob;

    int*  startOfUntriedSet = untriedSet;
    int** startOfOldUntriedSetEndStack = oldUntriedSetEndStack;
    int** startOfUntriedSetStack = untriedSetStack;

    if (initSteps <= 2) {       // base cases.
        if (initSteps <= 0) return 0;
        if (initSteps == 1) return 1 + COUNT_STEP_BELOW;

        neighbours = nodes[untriedSet[0]];
        counted = COUNT_STEP_BELOW;
        for (int i = 1; i <= NUM_OF_NEIGHBOURS; i++)
            counted += !nodesFound[neighbours[i]];      // TODO: maybe change to bool* isNew instead of bool* nodesFound (same same, but negative) for more optimizations.
        return counted;
    }

    bool mainBackup = access(backUpPath, F_OK ) != -1, tempBackup = access(tempBackUpPath, F_OK) != -1;
    if(BACKUP && (mainBackup || tempBackup)) {
        recover(mainBackup, backUpPath, tempBackUpPath, isNewJob,
                steps, counted, nextBackup, graphSize, nodesFound,
                startOfUntriedSet, untriedSetEnd,
                startOfOldUntriedSetEndStack, oldUntriedSetEndStack,
                startOfUntriedSetStack, untriedSetStack);
        goto continue_recovered;
    }

    new_call:
    neighbours = nodes[untriedSet[0]];  // [1] - get top node's neighbours from untried set (will be removed later)
                                        // [2] - we don't actually place a cell in our implementation
    if (steps == 2) {   // recursion stop condition
        // [3] - count the number of neighbours, because that's the number of polyominoes of size initSteps that could be constructed by the current (p-1)-polyomino
        counted += (untriedSetEnd-untriedSet) + (COUNT_STEP_BELOW - 1);     // count all neighbours in untriedSet (without itself, unless COUNT_STEP_BELOW is on)
        for (int i = 1; i <= NUM_OF_NEIGHBOURS; i++)    // count the current node neighbours, if they weren't been found yet.
            counted += !nodesFound[neighbours[i]];          // if neighbours[i] doesn't exists, it points to originCell, which was surely found.
        goto return_from_call;  // the 'return value' is counted value
    }

    *(untriedSetStack++) = untriedSet;          // backup untriedSet and untriedSetEnd
    *(oldUntriedSetEndStack++) = untriedSetEnd; // this will be oldUntriedSetEnd
    for (int i = 1; i <= NUM_OF_NEIGHBOURS; i++) {    // [4a] - add new neighbours to untried set and found set
        int newNode = neighbours[i];
        if (!nodesFound[newNode]) {
            *(untriedSetEnd++) = newNode;
            nodesFound[newNode] = true;
        }
    }

    // save current values
    if (steps == EXACT_FORK_LEVEL) {
        steps--;
        while (++untriedSet != untriedSetEnd)      // functions as while (...), and iterates over all possible untried set starting-points (first run - removes the top node, in reference to [1])
        {
            //prep_vars();
            nextBackup = BACKUP_INTERVALS;
            initSteps = steps;
            counted = 0;
            *(untriedSetStack++) = untriedSet;
            doBackup(backUpPath, tempBackUpPath, 1,
                     steps, counted, nextBackup, graphSize, nodesFound,
                     startOfUntriedSet, untriedSetEnd-startOfUntriedSet,
                     startOfOldUntriedSetEndStack, oldUntriedSetEndStack-startOfOldUntriedSetEndStack,
                     startOfUntriedSetStack, untriedSetStack-startOfUntriedSetStack);
            untriedSetStack--;
        }          // [4b] - call this algorithm recursively with the new untried set
        steps++;    // return from recursion call
    }
    // recover current values
    steps--;
    return_from_call:
    if (++untriedSet != untriedSetEnd)      // functions as while (...), and iterates over all possible untried set starting-points (first run - removes the top node, in reference to [1])
        goto new_call;      // [4b] - call this algorithm recursively with the new untried set
    steps++;    // return from recursion call

    oldUntriedSetEnd = *(--oldUntriedSetEndStack);
    while (oldUntriedSetEnd != untriedSetEnd)    // [4c] - remove all new neighbours from untried set and found set (and recover untriedSetEnd)
        nodesFound[*(--untriedSetEnd)] = false;

    if (BACKUP && (counted >= nextBackup)) {
        continue_recovered:;
        if (steps == initSteps)
            return counted;
    }
    untriedSet = *(--untriedSetStack);      // [5] and recover untriedSet (and therefore remove newest cell)
    if (steps < initSteps)
        goto return_from_call;  // the 'return value' is counted value

    goto continue_recovered;     // do final backup, and return counted;
}

///
/// \param nodes - the graph itself.
/// \param p - size of max sub-graphs.
/// \param n - number of vertices in graph.
/// \param originCell - starting node for any sub-graph.
/// \return - number of sub-graphs of 'nodes' contains at most 'p' nodes, including 'originCell'.
u64 countSubGraphs(int** nodes, int p, int n, int originCell, const char* backupName) {
    if (p <= 0) return 0;

    bool* nodesFound = (bool*)calloc(n, sizeof(bool));
    int* untriedSet = (int*)malloc(n * sizeof(int));
    nodesFound[originCell] = true;
    untriedSet[0] = originCell;

    char path[80], tempPath[80];
    char p_str[5];
    itoa(p, p_str, 10);
    strcpy(path, backupName);
    strcat(path, "_");
    if (p < 10) strcat(path, "0");
    strcat(path, p_str);
    strcat(path, ".txt");
    strcpy(tempPath, "_temp_");
    strcat(tempPath, path);

    int** oldUntriedSetEndStack = (int**)malloc(p * sizeof(int*));
    int** untriedSetStack = (int**)malloc(p * sizeof(int*));
    u64 count = recCounterGOTO(nodes, nodesFound, untriedSet, untriedSet+1, oldUntriedSetEndStack, untriedSetStack, p, n, path, tempPath);
    free(oldUntriedSetEndStack);
    free(untriedSetStack);

    free(nodesFound);
    free(untriedSet);
    return count;
}

///
/// \param p - max size of polyominoes.
/// \return - counts the number of fixed polyominoes up to the size of p.
u64 countPolyominoes(int p) {
    if (p < 1) return 0;
    int originCell, n;
    int** nodes = createPolyominoGraph(p, &originCell, &n);
    u64 count = countSubGraphs(nodes, p, n, originCell, "Polyomino");
    deleteGraph(nodes, originCell, 4);
    return count;
}


///
/// \param p - max size of polycubes.
/// \return - counts the number of fixed polycubes up to the size of p.
u64 countPolycubes(int p) {
    if (p < 1) return 0;
    int originCell, n;
    int** nodes = createPolycubesGraph(p, &originCell, &n);
    u64 count = countSubGraphs(nodes, p, n, originCell, "Polycube");
    deleteGraph(nodes, originCell, 6);
    return count;
}

///
/// \param p - max size of polyiamonds.
/// \return - counts the number of fixed polyiamonds up to the size of p,
///             including those start with left -pointing-triangle (count1),
///                   and those start with right-pointing-triangle (count2 - originCell-1)
u64 countPolyiamonds(int p) {
    if (p < 1) return 0;
    int originCell, n;
    int** nodes = createPolyiamondsGraph(p, &originCell, &n);
    u64 count = countSubGraphs(nodes, p, n, originCell, "Polyiamond");
    deleteGraph(nodes, originCell, 3);
    return count;

//    u64 count1 = countSubGraphs(nodes, p  , n, originCell, "Polyiamond_count1");
//    u64 count2 = countSubGraphs(nodes, p-1, n, originCell, "Polyiamond_count2");
//    deleteGraph(nodes, originCell, 3);
//    return count1 + count2;
}

///
/// \param graph
/// \param originCell
/// \param n
/// \param result
/// \param maxTime
/// \return
bool canIFinishIt(int** graph, int originCell, u32 steps, u32 numOfNodes, u64* result, double maxTime) {
    // Todo
    int pid = fork();
    pipe();
    if (pid == 0) {
        u64 counted = simpleCountSubGraphs(graph, steps, numOfNodes, originCell);
        put_results_in_pipe;
        exit();
    } else {
        sleep(maxTime);    // will be defined
        if (son_finished) {
            *results = get_sons_results_from_pipe;
            return true;
        }
        kill(pid);
        return false;
    }
}

u32 decideWhatLevel(int** graph, int originCell, u32 numOfNodes, u64 approxNumOfJobs, u64* numOfJobs) {
    // calculate
    u32 steps = 1;
    u64 lastResults=-1, results=-1;
    while (true) {
        if (canIFinishIt(graph, originCell, steps, numOfNodes, &results, 15.0) == false) {
            *numOfJobs = lastResults;
            return steps-1;
        } else if (results > approxNumOfJobs) {
            *numOfJobs = results;
            return steps;
        }
        lastResults = results;
        steps++;
    }

}

///
/// \param graph
/// \param n
/// \param level
/// \param numOfJobs
/// \param jobBaseName
/// \return 0 for success
int createJobs(int** graph, int originCell, u32 steps, u32 forkLevel, u64 numOfJobs, string jobBaseName) {    // jobBaseName + "_" + n + "_" + indexOfJob + ".job"
    countSubGraphs_calls_recCounterGOTO__and_create_jobs();
}

///
/// \param graph
/// \param n
/// \param approxNumOfJobs
/// \param jobBaseName
/// \return
int mainServer(string graphFilePath, u32 steps, int approxNumOfJobs, string jobBaseName) {
    u64 numOfJobs;
    int originCell, **graph;
    u32 numOfNodes;

    readGraphFromFile(graphFilePath, &originCell, &graph, &numOfNodes);
    u32 level = decideWhatLevel(graph, originCell, numOfNodes, approxNumOfJobs, &numOfJobs);
    int res = createJobs(graph, originCell, steps, steps - level, numOfJobs, jobBaseName);
    deleteGraph(graph, numOfNodes);
    return res;

    // return 0;
}
