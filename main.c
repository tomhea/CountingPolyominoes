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
/// \param originCellPtr -  output ptr for the origin cell of the graph.
/// \return - the created graph.
int** createPolyominoGraph(int p, int* originCellPtr, int* nPtr) {
    int originCell = p>=2 ? p-2 : 0;
    *originCellPtr = originCell;
    int height = p + originCell;
    int n = p*height;
    *nPtr = n;

    int** nodes = malloc(n * sizeof(int*));
    int* neighboursGlobal = malloc(n*5* sizeof(int));

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

void deleteGraph(int** nodes, int originCell) {
    int* neighboursGlobal = nodes[originCell] - 5*originCell;
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
    deleteGraph(nodes, originCell);
    return count;
}


int main() {
    u64 counted[30];
    clock_t times[30];
    counted[0] = 0;
    times[0] = clock();
    for(int i = 1; i < 30; i++) {
        counted[i] = countPolyominoes(i);
        times[i] = clock();
        printf("P(%2d) = %16llu  ~ %lds\n", i, counted[i]-counted[i - 1], (times[i]-times[i-1])/CLOCKS_PER_SEC);
    }
    return 0;
}
