#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

typedef unsigned long long u64;
#define BACKUP_INTERVALS 1000000

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

    int** nodes = malloc(n * sizeof(int*));
    int* neighboursGlobal = malloc(n*(1+4)* sizeof(int));

    for(int i = originCell; i < n; i++) {
        int* neighbours = neighboursGlobal + 5*i;
        int counter = 0;
        if (i - height >= originCell)
            neighbours[++counter] = i - height;
        if (i + height < n)
            neighbours[++counter] = i + height;
        if (((i%height) != 0) && (i != originCell))
            neighbours[++counter] = i-1;
        if (((i+1)%height) != 0)
            neighbours[++counter] = i+1;
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

    int** nodes = malloc(n * sizeof(int*));
    int* neighboursGlobal = malloc(n*(1+6)* sizeof(int));

    for(int i = originCell; i < n; i++) {
        int* neighbours = neighboursGlobal + 7*i;

        int counter = 0;
        if (i - plate >= originCell)
            neighbours[++counter] = i - plate;
        if (i + plate < n)
            neighbours[++counter] = i + plate;
        if (((i%height) != 0) && (i != originCell))
            neighbours[++counter] = i-1;
        if (((i+1)%height) != 0)
            neighbours[++counter] = i+1;
        if (i - height >= originCell && (i%plate)/height != 0)
            neighbours[++counter] = i - height;
        if (((i+height)%plate)/height != 0)
            neighbours[++counter] = i + height;
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

    int** nodes = malloc(n * sizeof(int*));
    int* neighboursGlobal = malloc(n*(1+3)* sizeof(int));

    for(int i = baseOriginCell; i < n; i++) {
        int* neighbours = neighboursGlobal + 4*i;
        int counter = 0;
        if (i % 2 == 0) {   // a right-pointing-triangle
            if (i - (height - 1) >= originCell)
                neighbours[++counter] = i - (height - 1);
        } else {            // a left -pointing-triangle
            if (i + (height - 1) < n)
                neighbours[++counter] = i + (height - 1);
        }
        if (((i%height) != 0) && (i > originCell))
            neighbours[++counter] = i-1;
        if (((i+1)%height) != 0)
            neighbours[++counter] = i+1;
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


void recover(const char* backUpFilePath,
             int* steps, u64* counted, u64* nextBackup, int* graphSize, bool* nodesFound,
             int* startOfUntriedSet, int** untriedSetEndPtr,
             int** startOfOldUntriedSetEndStack, int*** oldUntriedSetEndStackPtr,
             int** startOfUntriedSetStack, int*** untriedSetStackPtr) {
    FILE *f;
    f = fopen( backUpFilePath , "rb");

    fread(steps, sizeof(int), 1, f);
    fread(counted, sizeof(u64), 1, f);
    fread(nextBackup, sizeof(u64), 1, f);

    fread(graphSize, sizeof(int), 1, f);
    fread(nodesFound, sizeof(bool), *graphSize, f);

    int untriedSetEndIdx;
    fread(&untriedSetEndIdx, sizeof(int), 1, f);
    fread(startOfUntriedSet, sizeof(int), untriedSetEndIdx, f);
    *untriedSetEndPtr = startOfUntriedSet + untriedSetEndIdx;

    int* prevStartOfUntriedSet;
    fread(&prevStartOfUntriedSet, sizeof(int*), 1, f);

    int oldUntriedSetEndStackIdx;
    fread(&oldUntriedSetEndStackIdx, sizeof(int), 1, f);
    fread(startOfOldUntriedSetEndStack, sizeof(int*), oldUntriedSetEndStackIdx, f);
    *oldUntriedSetEndStackPtr = startOfOldUntriedSetEndStack + oldUntriedSetEndStackIdx;
    for (int i = 0; i < oldUntriedSetEndStackIdx; i++)
        startOfOldUntriedSetEndStack[i] = startOfUntriedSet + (startOfOldUntriedSetEndStack[i] - prevStartOfUntriedSet);

    int untriedSetStackIdx;
    fread(&untriedSetStackIdx, sizeof(int), 1, f);
    fread(startOfUntriedSetStack, sizeof(int*), untriedSetStackIdx, f);
    *untriedSetStackPtr = startOfUntriedSetStack + untriedSetStackIdx;
    for (int i = 0; i < untriedSetStackIdx; i++)
        startOfUntriedSetStack[i] = startOfUntriedSet + (startOfUntriedSetStack[i] - prevStartOfUntriedSet);

    fclose(f);
}

void doBackup(const char* backUpPath, const char* tempBackUpPath,
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
    int node, numOfNeighbours, *neighbours, *oldUntriedSetEnd;
    u64 counted = 0;
    u64 nextBackup = BACKUP_INTERVALS;
    int steps = initSteps;

    int* startOfUntriedSet = untriedSet;
    int** startOfOldUntriedSetEndStack = oldUntriedSetEndStack;
    int** startOfUntriedSetStack = untriedSetStack;

    if(access(backUpPath, F_OK ) != -1) {
        recover(backUpPath,
                &steps, &counted, &nextBackup, &graphSize, nodesFound,
                startOfUntriedSet, &untriedSetEnd,
                startOfOldUntriedSetEndStack, &oldUntriedSetEndStack,
                startOfUntriedSetStack, &untriedSetStack);
        goto continue_recovered;
    } else if (access(tempBackUpPath, F_OK) != -1) {
        recover(tempBackUpPath,
                &steps, &counted, &nextBackup, &graphSize, nodesFound,
                startOfUntriedSet, &untriedSetEnd,
                startOfOldUntriedSetEndStack, &oldUntriedSetEndStack,
                startOfUntriedSetStack, &untriedSetStack);
        rename(tempBackUpPath, backUpPath);
        goto continue_recovered;
    }

    new_call:
    *(untriedSetStack++) = untriedSet;
    node = *(untriedSet++);   // pop node from untried set
    neighbours = nodes[node];
    numOfNeighbours = neighbours[0];

    if (steps == 2) {   // recursion stop condition
        oldUntriedSetEnd = untriedSetEnd;
        untriedSetEnd += numOfNeighbours;
        for (int i = 1; i <= numOfNeighbours; i++)
            untriedSetEnd -= nodesFound[neighbours[i]];
        counted += 1 + untriedSetEnd-untriedSet;
        untriedSetEnd = oldUntriedSetEnd;
    } else {
        *(oldUntriedSetEndStack++) = untriedSetEnd;     // initialize oldUntriedSetEnd
        for (int i = 1; i <= numOfNeighbours; i++) {    // add new neighbours to untried set and found set
            int newNode = neighbours[i];
            if (!nodesFound[newNode]) {
                *(untriedSetEnd++) = newNode;
                nodesFound[newNode] = true;
            }
        }

        steps--;
        counted++;      // count current sub-graph.
        while (untriedSet != untriedSetEnd) {
            goto new_call;
        return_from_call:
            untriedSet++;
        }
        steps++;

        oldUntriedSetEnd = *(--oldUntriedSetEndStack);
        while (oldUntriedSetEnd != untriedSetEnd)    // remove all new neighbours from found set.
            nodesFound[*(--untriedSetEnd)] = false;

        if (counted >= nextBackup) {
            nextBackup += BACKUP_INTERVALS;
            doBackup(backUpPath, tempBackUpPath, steps, counted, nextBackup, graphSize, nodesFound,
                    startOfUntriedSet, untriedSetEnd-startOfUntriedSet,
                    startOfOldUntriedSetEndStack, oldUntriedSetEndStack-startOfOldUntriedSetEndStack,
                    startOfUntriedSetStack, untriedSetStack-startOfUntriedSetStack);
            continue_recovered:;
        }
    }
    untriedSet = *(--untriedSetStack);
    if (steps < initSteps)
        goto return_from_call;  // the 'return value' is counted value


    return counted;
}

///
/// \param nodes - the graph itself.
/// \param p - size of max sub-graphs.
/// \param n - number of vertices in graph.
/// \param originCell - starting node for any sub-graph.
/// \return - number of sub-graphs of 'nodes' contains at most 'p' nodes, including 'originCell'.
u64 countSubGraphs(int** nodes, int p, int n, int originCell, const char* backupName) {
    if (p <= 1) return p == 1;

    bool* nodesFound = calloc(n, sizeof(bool));
    int* untriedSet = malloc(n * sizeof(int));
    nodesFound[originCell] = true;
    untriedSet[0] = originCell;

    char path[80], tempPath[80];
    char p_str[5];
    itoa(p, p_str, 10);
    strcpy(path, backupName);
    strcat(path, "_");
    strcat(path, p_str);
    strcat(path, ".txt");
    strcpy(tempPath, "_temp_");
    strcat(tempPath, path);

//    u64* countedStack = malloc(p * sizeof(u64));
    int** oldUntriedSetEndStack = malloc(p * sizeof(int*));
    int** untriedSetStack = malloc(p * sizeof(int*));
    u64 count = recCounterGOTO(nodes, nodesFound, untriedSet, untriedSet+1, oldUntriedSetEndStack, untriedSetStack, p, n, path, tempPath);
//    free(countedStack);
    free(oldUntriedSetEndStack);
    free(untriedSetStack);

    // u64 count = recCounter(nodes, p, nodesFound, untriedSet, untriedSet+1)

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
u64 countPolyiamonds(int p, u64* counts) {
    if (p < 1) return 0;
    int originCell, n;
    int** nodes = createPolyiamondsGraph(p, &originCell, &n);
    u64 count1 = countSubGraphs(nodes, p, n, originCell, "Polyiamond_count1"), count2;
    if (counts) {
        count2 = 1 + counts[p-1]; // count2 is same as count1 of previous p, the difference is an added baseOrigin
        counts[p] = count1;
    } else {
        count2 = countSubGraphs(nodes, p, n, originCell-1, "Polyiamond_count2");
    }
    deleteGraph(nodes, originCell, 3);
    return count1 + count2;
}


int main() {
    u64 counted[30];
    clock_t times[30];
    counted[0] = 0;
    times[0] = clock();

    // Used only for polyiamonds
    u64 helpCounts[30];
    helpCounts[0] = 0;

    for(int i = 1; i < 30; i++) {
//        counted[i]=countPolyiamonds(i,helpCounts);
//        counted[i]=countPolycubes(i);
        counted[i]=countPolyominoes(i);
        times[i] = clock();
        printf("P(%2d) = %16llu  ~ %lds\n", i, counted[i]-counted[i - 1], (times[i]-times[i-1])/CLOCKS_PER_SEC);
    }
    return 0;
}
