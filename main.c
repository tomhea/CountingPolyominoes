#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

typedef unsigned long long u64;

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

///
/// \param nodes - the graph itself.
/// \param p - size of max sub-graphs.
/// \param n - number of vertices in graph.
/// \param originCell - starting node for any sub-graph.
/// \return - number of sub-graphs of 'nodes' contains at most 'p' nodes, including 'originCell'.
u64 countSubGraphs(int** nodes, int p, int n, int originCell) {
    if (p <= 1) return p == 1;

    bool* nodesFound = calloc(n, sizeof(bool));
    int* untriedSet = malloc(n * sizeof(int));
    nodesFound[originCell] = true;
    untriedSet[0] = originCell;

    u64 count = recCounter(nodes, p, nodesFound, untriedSet, untriedSet+1);

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
    u64 count = countSubGraphs(nodes, p, n, originCell);
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
    u64 count = countSubGraphs(nodes, p, n, originCell);
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
    u64 count1 = countSubGraphs(nodes, p, n, originCell);
    //u64 count2 = countSubGraphs(nodes, p, n, --originCell);
    u64 count2 = 1 + counts[p-1]; // count2 is same as count1 of previous p, the difference is an added baseOrigin
    deleteGraph(nodes, originCell, 3);
    counts[p] = count1;
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
        counted[i]=countPolyiamonds(i,helpCounts);
        times[i] = clock();
        printf("P(%2d) = %16llu  ~ %lds\n", i, counted[i]-counted[i - 1], (times[i]-times[i-1])/CLOCKS_PER_SEC);
    }
    return 0;
}
