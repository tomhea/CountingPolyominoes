#include "redelServer.h"
#include "graphCreator.h"
#include "backups.h"


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
    while(untriedSet != untriedSetEnd) {
        counted += recCounter(nodes, steps - 1, nodesFound, untriedSet++, untriedSetEnd);
        if (counted > COUNTER_THRESHOLD) throw recTimeout();    // if passed the threshold - QUIT.
    }

    while(oldUntriedSetEnd != untriedSetEnd)    // remove all new neighbours from found set.
        nodesFound[*(--untriedSetEnd)] = false;

    return counted;
}


///
/// \param nodes - the graph itself.
/// \param steps - size of max sub-graphs.
/// \param numOfNodes - number of vertices in graph.
/// \param originCell - starting node for any sub-graph.
/// \return - number of sub-graphs of 'nodes' contains at most 'steps' nodes, including 'originCell'.
bool simpleTimedCountSubGraphs(int** nodes, u32 steps, u32 numOfNodes, int originCell, u64* count) {
    if (steps <= 1) return steps;

    bool* nodesFound = (bool*)calloc(numOfNodes, sizeof(bool));
    int* untriedSet = (int*)malloc(numOfNodes * sizeof(int));
    nodesFound[originCell] = true;
    untriedSet[0] = originCell;

    bool success;
    try {
        *count = recCounter(nodes, steps, nodesFound, untriedSet, untriedSet + 1);
        success = true;
    } catch (recTimeout& e) {
        *count = 0;
        success = false;
    }

    free(nodesFound);
    free(untriedSet);

    return success;
}


///
/// \param nodes - the graph itself.
/// \param steps - number of nodes to add to the sub-graph.
/// \param nodesFound - boolean array of whether a node is in the sub-graph / untriedSet, or isn't.
/// \param untriedSet - array of size 'untriedSize' containing all the reachable-but-not-used of the sub-graph.
/// \param untriedSize - number of elements in the untried set.
/// \return - number of sub-graphs
u64 recJobsCreatorGOTO(int** nodes, bool* nodesFound, int* untriedSet, int* untriedSetEnd, int** oldUntriedSetEndStack, int** untriedSetStack, u32 initSteps, u32 graphSize, u32 forkLevel, char* jobPath, const char* tempJobPath) {
    int *neighbours, *oldUntriedSetEnd;
    u32 steps = initSteps;

    u32 jobId = 0;
    char jobIdStr[20];
    u64 jobPathLen = strlen(jobPath);
    jobPath[jobPathLen++] = '_';

    int*  startOfUntriedSet = untriedSet;
    int** startOfOldUntriedSetEndStack = oldUntriedSetEndStack;
    int** startOfUntriedSetStack = untriedSetStack;

    new_call:
    neighbours = nodes[untriedSet[0]];  // [1] - get top node's neighbours from untried set (will be removed later)
                                        // [2] - we don't actually place a cell in our implementation

    *(untriedSetStack++) = untriedSet;          // backup untriedSet and untriedSetEnd
    *(oldUntriedSetEndStack++) = untriedSetEnd; // this will be oldUntriedSetEnd
    for (int i = 1; i <= neighbours[0]; i++) {    // [4a] - add new neighbours to untried set and found set
        int newNode = neighbours[i];
        if (!nodesFound[newNode]) {
            *(untriedSetEnd++) = newNode;
            nodesFound[newNode] = true;
        }
    }

    steps--;
    if (steps == forkLevel) {
        while (++untriedSet != untriedSetEnd) {    // functions as while (...), and iterates over all possible untried set starting-points (first run - removes the top node, in reference to [1])
            *(untriedSetStack++) = untriedSet;
            itoa(jobId++, jobIdStr, 10);
            strcpy(jobPath + jobPathLen, jobIdStr);
            doBackup(jobPath, tempJobPath, 1,
                     steps, steps, 0, BACKUP_INTERVALS, graphSize, nodesFound,
                     startOfUntriedSet, untriedSetEnd-startOfUntriedSet,
                     startOfOldUntriedSetEndStack, oldUntriedSetEndStack-startOfOldUntriedSetEndStack,
                     startOfUntriedSetStack, untriedSetStack-startOfUntriedSetStack);
            untriedSetStack--;
        }          // [4b] - call this algorithm recursively with the new untried set
    } else {
        return_from_call:
        if (++untriedSet != untriedSetEnd)      // functions as while (...), and iterates over all possible untried set starting-points (first run - removes the top node, in reference to [1])
            goto new_call;      // [4b] - call this algorithm recursively with the new untried set
    }
    steps++;    // return from recursion call

    oldUntriedSetEnd = *(--oldUntriedSetEndStack);
    while (oldUntriedSetEnd != untriedSetEnd)    // [4c] - remove all new neighbours from untried set and found set (and recover untriedSetEnd)
        nodesFound[*(--untriedSetEnd)] = false;

    untriedSet = *(--untriedSetStack);      // [5] and recover untriedSet (and therefore remove newest cell)
    if (steps < initSteps)
        goto return_from_call;  // the 'return value' is counted value

    return jobId;
}


bool canIFinishIt(const char* graphFilePath, u32 steps, u64* result) {
    int originCell, **graph;
    u32 numOfNodes;

    readGraphFromFile(graphFilePath, &originCell, &graph, &numOfNodes);
    return simpleTimedCountSubGraphs(graph, steps, numOfNodes, originCell, result);
}

u32 decideWhatLevel(int** graph, int originCell, u32 numOfNodes, u64 approxNumOfJobs, u64* numOfJobs) {
    // calculate
    u32 steps = 1;
    u64 lastResults=-1, results=-1;
    while (true) {
        if (simpleTimedCountSubGraphs(graph, steps, numOfNodes, originCell, &results) == false) {
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
u64 recJobsCreatorWrapper(int** nodes, int originCell, u32 numOfNodes, u32 steps, u32 forkLevel, const char* jobBasePath) {    // jobBaseName + "_" + n + "_" + indexOfJob + ".job"
    if (steps <= 1) return steps;

    bool* nodesFound = (bool*)calloc(numOfNodes, sizeof(bool));
    int* untriedSet = (int*)malloc(numOfNodes * sizeof(int));
    int** oldUntriedSetEndStack = (int**)malloc(steps * sizeof(int*));
    int** untriedSetStack = (int**)malloc(steps * sizeof(int*));

    nodesFound[originCell] = true;
    untriedSet[0] = originCell;

    char path[MAX_PATH_LEN], tempPath[MAX_PATH_LEN];
    strcpy(path, jobBasePath);
    strcpy(tempPath, jobBasePath);
    strcat(tempPath, TEMP_FILE_SUFFIX);

    u64 jobsCreated = recJobsCreatorGOTO(nodes, nodesFound, untriedSet, untriedSet + 1, oldUntriedSetEndStack,
                                         untriedSetStack, steps, numOfNodes, forkLevel, path, tempPath);

    free(nodesFound);
    free(untriedSet);
    free(oldUntriedSetEndStack);
    free(untriedSetStack);
    return jobsCreated;
}

///
/// \param graph
/// \param n
/// \param approxNumOfJobs
/// \param jobBasePath
/// \return
int jobsCreator(const char* graphFilePath, u32 steps, int approxNumOfJobs, const char* jobBasePath) {
    u64 numOfJobs;
    int originCell, **graph;
    u32 numOfNodes;

    readGraphFromFile(graphFilePath, &originCell, &graph, &numOfNodes);
    u32 level = decideWhatLevel(graph, originCell, numOfNodes, approxNumOfJobs, &numOfJobs);
    u64 jobsCreated = recJobsCreatorWrapper(graph, originCell, numOfNodes, steps, steps - (level-1), jobBasePath);
    assert(jobsCreated == numOfJobs);
    deleteGraph(graph, numOfNodes);
    return jobsCreated;

    // return 0;
}
