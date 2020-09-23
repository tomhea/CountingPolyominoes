#include "redelClient.h"
#include "graphCreator.h"
#include "backups.h"


///
/// \param nodes - the graph itself.
/// \param steps - number of nodes to add to the sub-graph.
/// \param nodesFound - boolean array of whether a node is in the sub-graph / untriedSet, or isn't.
/// \param untriedSet - array of size 'untriedSize' containing all the reachable-but-not-used of the sub-graph.
/// \param untriedSize - number of elements in the untried set.
/// \return - number of sub-graphs
u64 recCounterGOTO(int** nodes, bool* nodesFound, int* untriedSet, int** oldUntriedSetEndStack, int** untriedSetStack, const char* backUpPath, const char* tempBackUpPath) {
    int *neighbours, *oldUntriedSetEnd;
    u64 counted;
    u64 nextBackup;
    u32 steps, initSteps;
    int isNewJob;

    int*  startOfUntriedSet;
    int** startOfOldUntriedSetEndStack;
    int** startOfUntriedSetStack;

    int* untriedSetEnd;
    u32 graphSize;

    bool mainBackup = access(backUpPath, F_OK ) != -1, tempBackup = access(tempBackUpPath, F_OK) != -1;
    assert(mainBackup || tempBackup);
    recover(mainBackup, backUpPath, tempBackUpPath, isNewJob,
            initSteps, steps, counted, nextBackup, graphSize, nodesFound,
            startOfUntriedSet, untriedSetEnd,
            startOfOldUntriedSetEndStack, oldUntriedSetEndStack,
            startOfUntriedSetStack, untriedSetStack);
    if (isNewJob == 0)
        goto continue_recovered;    // continue from local backup

                                    // otherwise - it's a new job from the server
    untriedSet = *(--untriedSetStack);

    // recursion stop conditions
    if (initSteps <= 1) return initSteps;
    if (initSteps == 2) {
        // [3] - count the number of neighbours, because that's the number of polyominoes of size initSteps that could be constructed by the current (p-1)-polyomino
        counted += (untriedSetEnd-untriedSet) - 1;     // count all neighbours in untriedSet (without itself, unless COUNT_STEP_BELOW is on)
        for (int i = 1; i <= NUM_OF_NEIGHBOURS; i++)    // count the current node neighbours, if they weren't been found yet.
            counted += !nodesFound[neighbours[i]];          // if neighbours[i] doesn't exists, it points to originCell, which was surely found.
        return counted;
    }



    new_call:
    neighbours = nodes[untriedSet[0]];  // [1] - get top node's neighbours from untried set (will be removed later)
                                        // [2] - we don't actually place a cell in our implementation
    if (steps == 2) {   // recursion stop condition
        // [3] - count the number of neighbours, because that's the number of polyominoes of size initSteps that could be constructed by the current (p-1)-polyomino
        counted += (untriedSetEnd-untriedSet) - 1;     // count all neighbours in untriedSet (without itself, unless COUNT_STEP_BELOW is on)
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

    steps--;
    return_from_call:
    if (++untriedSet != untriedSetEnd)      // functions as while (...), and iterates over all possible untried set starting-points (first run - removes the top node, in reference to [1])
        goto new_call;      // [4b] - call this algorithm recursively with the new untried set
    steps++;    // return from recursion call

    oldUntriedSetEnd = *(--oldUntriedSetEndStack);
    while (oldUntriedSetEnd != untriedSetEnd)    // [4c] - remove all new neighbours from untried set and found set (and recover untriedSetEnd)
        nodesFound[*(--untriedSetEnd)] = false;

    if (counted >= nextBackup) {
        do_backup:
        nextBackup += BACKUP_INTERVALS;
        doBackup(backUpPath, tempBackUpPath, 0,
                initSteps, steps, counted, nextBackup, graphSize, nodesFound,
                startOfUntriedSet, untriedSetEnd-startOfUntriedSet,
                startOfOldUntriedSetEndStack, oldUntriedSetEndStack-startOfOldUntriedSetEndStack,
                startOfUntriedSetStack, untriedSetStack-startOfUntriedSetStack);
        continue_recovered:;
        if (steps == initSteps)
            return counted;
    }
    untriedSet = *(--untriedSetStack);      // [5] and recover untriedSet (and therefore remove newest cell)
    if (steps < initSteps)
        goto return_from_call;  // the 'return value' is counted value

    goto do_backup;     // do final backup, and return counted;
}

///
/// \param nodes - the graph itself.
/// \param p - size of max sub-graphs.
/// \param numOfNodes - number of vertices in graph.
/// \param originCell - starting node for any sub-graph.
/// \return - number of sub-graphs of 'nodes' contains at most 'p' nodes, including 'originCell'.
u64 countSubGraphs(int** nodes, u32 numOfNodes, int originCell, const char* jobBasePath) {
    bool* nodesFound = (bool*)calloc(numOfNodes, sizeof(bool));
    int* untriedSet = (int*)malloc(numOfNodes * sizeof(int));
    int** oldUntriedSetEndStack = (int**)malloc(numOfNodes * sizeof(int*));   // numOfNodes can be replaced with initSteps
    int** untriedSetStack = (int**)malloc(numOfNodes * sizeof(int*));         // numOfNodes can be replaced with initSteps

    char path[MAX_PATH_LEN], tempPath[MAX_PATH_LEN];
    strcpy(path, jobBasePath);
    strcpy(tempPath, jobBasePath);
    strcat(tempPath, TEMP_FILE_SUFFIX);

    u64 count = recCounterGOTO(nodes, nodesFound, untriedSet, oldUntriedSetEndStack, untriedSetStack, path, tempPath);

    free(nodesFound);
    free(untriedSet);
    free(oldUntriedSetEndStack);
    free(untriedSetStack);
    return count;
}


int mainClient(const char* graphFilePath, const char* jobBasePath) {
    int originCell, **graph;
    u32 numOfNodes;
    readGraphFromFile(graphFilePath, &originCell, &graph, &numOfNodes);
    return countSubGraphs(graph, numOfNodes, originCell, jobBasePath);
}
