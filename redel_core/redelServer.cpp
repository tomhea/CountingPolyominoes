#include "redelServer.h"
#include "graphCreator.h"
#include "backups.h"

/* A utility function to reverse a string  */
void reverse(char str[], int length)
{
    int start = 0;
    int end = length -1;
    while (start < end)
    {
        swap(*(str+start), *(str+end));
        start++;
        end--;
    }
}

// Implementation of itoa()
char* itoa(int num, char* str, int base) {
    int i = 0;
    bool isNegative = false;

    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // In standard itoa(), negative numbers are handled only with
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10) {
        isNegative = true;
        num = -num;
    }

    // Process individual digits
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }

    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';

    str[i] = '\0'; // Append string terminator

    // Reverse the string
    reverse(str, i);

    return str;
}

///
/// \param nodes - the graph itself.
/// \param steps - number of nodes to add to the sub-graph.
/// \param nodesFound - boolean array of whether a node is in the sub-graph / untriedSet, or isn't.
/// \param untriedSet - array of size 'untriedSize' containing all the reachable-but-not-used of the sub-graph.
/// \param untriedSize - number of elements in the untried set.
/// \return - number of sub-graphs
u64 recCounter(int** nodes, u32 steps, bool* nodesFound, int* untriedSet, int* untriedSetEnd) {
    int node = *(untriedSet);   // pop node from untried set
    int* neighbours = nodes[node];
    int numOfNeighbours = neighbours[0];

    if (steps == 2) {   // recursion stop condition
        // [3] - count the number of neighbours, because that's the number of polyominoes of size initSteps that could be constructed by the current (p-1)-polyomino
        u64 counted = (untriedSetEnd-untriedSet) - 1;     // count all neighbours in untriedSet (without itself, unless COUNT_STEP_BELOW is on)
        for (int i = 1; i <= neighbours[0]; i++)    // count the current node neighbours, if they weren't been found yet.
            counted += !nodesFound[neighbours[i]];          // if neighbours[i] doesn't exists, it points to originCell, which was surely found.
        return counted;
    }

    int* oldUntriedSetEnd = untriedSetEnd;
    for (int i = 1; i <= numOfNeighbours; i++) {    // add new neighbours to untried set and found set
        int newNode = neighbours[i];
        if (!nodesFound[newNode]) {
            *(untriedSetEnd++) = newNode;
            nodesFound[newNode] = true;
        }
    }

    u64 counted = 0;        // count current sub-graph.
    while(++untriedSet != untriedSetEnd) {
        counted += recCounter(nodes, steps - 1, nodesFound, untriedSet, untriedSetEnd);
        if (counted >= COUNTER_THRESHOLD) return counted;    // if passed the threshold - QUIT (recursively returns with value larger than COUNTER_THRESHOLD).
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
    if (steps <= 1) {
        *count = steps;
        return true;
    }

    bool* nodesFound = (bool*)calloc(numOfNodes, sizeof(bool));
    int* untriedSet = (int*)malloc(numOfNodes * sizeof(int));
    nodesFound[originCell] = true;
    untriedSet[0] = originCell;

    *count = recCounter(nodes, steps, nodesFound, untriedSet, untriedSet + 1);
    bool success = *count < COUNTER_THRESHOLD;      // if not passed the threshold
    if (!success) *count = 0;

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

    if (steps == forkLevel) {
        *(untriedSetStack++) = untriedSet;
        itoa(jobId++, jobIdStr, 10);
        strcpy(jobPath + jobPathLen, jobIdStr);
        doBackup(jobPath, tempJobPath, 1,
                 steps, steps, 0, BACKUP_INTERVALS, graphSize, nodesFound,
                 startOfUntriedSet, untriedSetEnd-startOfUntriedSet,
                 startOfOldUntriedSetEndStack, oldUntriedSetEndStack-startOfOldUntriedSetEndStack,
                 startOfUntriedSetStack, untriedSetStack-startOfUntriedSetStack);
        return 1;
    }

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
        while (++untriedSet != untriedSetEnd) {
            // functions as while (...), and iterates over all possible untried set starting-points (first run - removes the top node, in reference to [1])
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
    u32 steps = 1;
    u64 lastResults=-1, results=-1;
    while (true) {
        if (simpleTimedCountSubGraphs(graph, steps, numOfNodes, originCell, &results) == false) {
            *numOfJobs = lastResults;
            return steps-1;
        }
        if (results >= approxNumOfJobs) {
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
    deleteGraph(graph, numOfNodes);
    return jobsCreated;
}
