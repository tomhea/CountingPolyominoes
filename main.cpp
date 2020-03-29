#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <ctime>

typedef unsigned int u32;   // TODO: move whatever we can to unsigned
typedef unsigned long long u64;
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


/*
 * Graphs in this project are represented as list of int* (node),
 *  each node points to an int-array (the node's neighbours).
 *   In the first index there is the number of neighbours, and after it the neighbours themselves (a node is represented as an int).
 */

//typedef struct _list {
//    int value;
//    void* next;
//} * List;

/*  _ _ _ _
 * |_|_|_|_|
 * |_|_|_|_|
 * |_|_|_|_|
 * |S|_|_|_|
 *   |_|_|_|
 *   |_|_|_|
 *
 *  Example for p=4 graph. originCell is 'S'.
*/
/// \param p - max number of movement in the graph, from the origin cell.
/// \param originCellPtr - output ptr for the origin cell of the graph.
/// \param nPtr - output ptr for the number of vertices in the graph.
/// \return - the created graph.
int** createPolyominoGraph(int p, int* originCellPtr, int* nPtr) {
    int originCell = p>=2 ? p-2 : 0;
    *originCellPtr = originCell;
    int height = p + originCell;
    int n = p*height;
    *nPtr = n;

    int** nodes = (int**)malloc(n * sizeof(int*));
    int* neighboursGlobal = (int*)malloc(n*(1+4)* sizeof(int));

    for(int i = originCell; i < n; i++) {   // TODO: delete the four inner 'else' if want normal (but slower) graph
        int* neighbours = neighboursGlobal + 5*i;
        int counter = 0;
        if (i - height >= originCell)
            neighbours[++counter] = i - height;
        else neighbours[++counter] = originCell;
        if (i + height < n)
            neighbours[++counter] = i + height;
        else neighbours[++counter] = originCell;
        if (((i%height) != 0) && (i != originCell))
            neighbours[++counter] = i-1;
        else neighbours[++counter] = originCell;
        if (((i+1)%height) != 0)
            neighbours[++counter] = i+1;
        else neighbours[++counter] = originCell;
        neighbours[0] = counter;
        nodes[i] = neighbours;
    }
    return nodes;
}

///
/// \param p - max number of movement in the graph, from the origin cell.
/// \param originCellPtr - output ptr for the origin cell of the graph.
/// \param nPtr - output ptr for the number of vertices in the graph.
/// \return - the created graph.
int** createPolycubesGraph(int p, int* originCellPtr, int* nPtr) {
    int originCellY = p>=2 ? (p-2) : 0;
    int originCellZ = p>=2 ? (p-2) : 0;
    int height = p+originCellZ;
    int length = p+originCellY;
    int plate = height*length;
    int originCell = originCellY*height + originCellZ;
    *originCellPtr = originCell;
    int n = p*plate;
    *nPtr = n;

    int** nodes = (int**)malloc(n * sizeof(int*));              // TODO: change to cpp initializations in all file (and maybe use struct for nodes, like: {numOfNeighs, neighs[4]})
    int* neighboursGlobal = (int*)malloc(n*(1+6)* sizeof(int));

    for(int i = originCell; i < n; i++) {   // TODO: delete the six inner 'else' if want normal (but slower) graph
        int* neighbours = neighboursGlobal + 7*i;

        int counter = 0;
        if (i - plate >= originCell)
            neighbours[++counter] = i - plate;
        else neighbours[++counter] = originCell;
        if (i + plate < n)
            neighbours[++counter] = i + plate;
        else neighbours[++counter] = originCell;
        if (((i%height) != 0) && (i != originCell))
            neighbours[++counter] = i-1;
        else neighbours[++counter] = originCell;
        if (((i+1)%height) != 0)
            neighbours[++counter] = i+1;
        else neighbours[++counter] = originCell;
        if (i - height >= originCell && (i%plate)/height != 0)
            neighbours[++counter] = i - height;
        else neighbours[++counter] = originCell;
        if (((i+height)%plate)/height != 0)
            neighbours[++counter] = i + height;
        else neighbours[++counter] = originCell;
        neighbours[0] = counter;
        nodes[i] = neighbours;
    }
    return nodes;
}

///
/// \param p - max number of movement in the graph, from the origin cell.
/// \param originCellPtr - output ptr for the origin cell of the graph.
/// \param nPtr - output ptr for the number of vertices in the graph.
/// \return - the created graph.
/// Counting with the polyiamonds graph should be done twice -
///     one with 'baseOriginCell' as the origin cell,
///     and one without the 'baseOriginCell' while using the cell above it 'originCell' as the origin cell.
/// starting from 'originCell', it's impossible to get to 'baseOriginCell' -
///     so running the algorithm as mentioned above is enough.
int** createPolyiamondsGraph(int p, int* originCellPtr, int* nPtr) {
    int baseOriginCell = (p >= 2 ? p - 2 : 0);
    baseOriginCell += baseOriginCell % 2;
    int originCell = baseOriginCell + 1;
    *originCellPtr = originCell;
    int height = p + originCell;
    height += height % 2;
    int length = p/2 + 1;
    int n = height*length;
    *nPtr = n;

    int** nodes = (int**)malloc(n * sizeof(int*));
    int* neighboursGlobal = (int*)malloc(n*(1+3)* sizeof(int));

    for(int i = baseOriginCell; i < n; i++) {   // TODO: delete the four inner 'else' if want normal (but slower) graph
        int* neighbours = neighboursGlobal + 4*i;
        int counter = 0;
        if (i % 2 == 0) {   // a right-pointing-triangle
            if (i - (height - 1) >= originCell)
                neighbours[++counter] = i - (height - 1);
            else neighbours[++counter] = originCell;
        } else {            // a left -pointing-triangle
            if (i + (height - 1) < n)
                neighbours[++counter] = i + (height - 1);
            else neighbours[++counter] = originCell;
        }
        if (((i%height) != 0) && (i > originCell))
            neighbours[++counter] = i-1;
        else neighbours[++counter] = originCell;
        if (((i+1)%height) != 0)
            neighbours[++counter] = i+1;
        else neighbours[++counter] = originCell;
        neighbours[0] = counter;
        nodes[i] = neighbours;
    }
    return nodes;
}

void deleteGraph(int** nodes, int originCell, int neighboursSize) {
    int* neighboursGlobal = nodes[originCell] - (neighboursSize+1)*originCell;
    free(nodes);
    free(neighboursGlobal);
}




///
/// \param nodes - the graph itself.
/// \param steps - number of nodes to add to the sub-graph.
/// \param nodesFound - boolean array of whether a node is in the sub-graph / untriedSet, or isn't.
/// \param untriedSet - array of size 'untriedSize' containing all the reachable-but-not-used of the sub-graph.
/// \param untriedSize - number of elements in the untried set.
/// \return - number of sub-graphs
u64 recCounter(int** nodes, int steps, bool* nodesFound, int* untriedSet, int* untriedSetEnd) {
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


inline void recover(bool mainBackup, const char* backUpPath, const char* tempBackUpPath,
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

inline void doBackup(const char* backUpPath, const char* tempBackUpPath,
        int steps, u64 counted, u64 nextBackup, int graphSize, bool* nodesFound,
        int* startOfUntriedSet, int untriedSetEndIdx,
        int** startOfOldUntriedSetEndStack, int oldUntriedSetEndStackIdx,
        int** startOfUntriedSetStack, int untriedSetStackIdx) {
    FILE *f;
    f = fopen(tempBackUpPath , "wb");

    fwrite(&steps, sizeof(int), 1, f);
    fwrite(&counted, sizeof(u64), 1, f);
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
        recover(mainBackup, backUpPath, tempBackUpPath,
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

    steps--;
    return_from_call:
    if (++untriedSet != untriedSetEnd)      // functions as while (...), and iterates over all possible untried set starting-points (first run - removes the top node, in reference to [1])
        goto new_call;          // [4b] - call this algorithm recursively with the new untried set
    steps++;    // return from recursion call

    oldUntriedSetEnd = *(--oldUntriedSetEndStack);
    while (oldUntriedSetEnd != untriedSetEnd)    // [4c] - remove all new neighbours from untried set and found set (and recover untriedSetEnd)
        nodesFound[*(--untriedSetEnd)] = false;

    if (BACKUP && (counted >= nextBackup)) {
        do_backup:
        nextBackup += BACKUP_INTERVALS;
        doBackup(backUpPath, tempBackUpPath,
                steps, counted, nextBackup, graphSize, nodesFound,
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


int main() {
    int N = 35;
    u64 counted[N];
    clock_t times[N];
    counted[0] = 0;
    times[0] = clock();

    // Used only for polyiamonds

    for(int i = 1; i < N; i++) {
        #ifdef Polyiamonds
        counted[i] = countPolyiamonds(i);
        #endif
        #ifdef Polyominoes
        counted[i] = countPolyominoes(i);
        #endif
        #ifdef Polycubes
        counted[i] = countPolycubes(i);
        #endif
//        counted[i] = countPolyominoes(i);
        times[i] = clock();
        printf("P(%2d) = %16llu  ~ %lds\n", i, counted[i], (times[i]-times[i-1])/CLOCKS_PER_SEC);
    }
    return 0;
}
