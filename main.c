#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


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

void deleteGraph(int** nodes, int p, int originCell) {
    int* neighboursGlobal = nodes[originCell] - 5*originCell;
    free(nodes);
    free(neighboursGlobal);
}

int recCounter(int** nodes, int steps, bool* nodesFound, int* untriedSet, int untriedSize) {
    /*if (steps == 0)
        return 0;   // TODO: if (steps == 1): return untriedSize;*/

    if (steps == 1)
        return 1;

    int node = untriedSet[0];
    untriedSet++;
    untriedSize--;
    int counted = 1;
    int* oldEndOfUntriedSet = untriedSet + untriedSize;

    int* neighbours = nodes[node];
    int numOfNeighbours = neighbours[0];

    for (int i = 1; i <= numOfNeighbours; i++) {
        int newNode = neighbours[i];
        if (!nodesFound[newNode]) {
            untriedSet[untriedSize++] = newNode;
            nodesFound[newNode] = true;
        }
    }

    if (steps == 2) {
        untriedSet += untriedSize;
        while(oldEndOfUntriedSet != untriedSet--)
            nodesFound[untriedSet[0]] = false;
        return 1 + untriedSize;
    }

    while(untriedSize)
        counted += recCounter(nodes, steps-1, nodesFound, untriedSet++, untriedSize--);

    while(oldEndOfUntriedSet != untriedSet--)
        nodesFound[untriedSet[0]] = false;

    return counted;
}

int countSubGraphs(int** nodes, int p, int n, int originCell) {
    bool* nodesFound = calloc(n, sizeof(bool));
    int* untriedSet = malloc(n * sizeof(int));
    nodesFound[originCell] = true;
    untriedSet[0] = originCell;

    int count = recCounter(nodes, p, nodesFound, untriedSet, 1);

    free(nodesFound);
    free(untriedSet);
    return count;
}

int countPolyominoes(int p) {
    if (p < 1) return 0;
    int originCell, n;
    int** nodes = createPolyominoGraph(p, &originCell, &n);
    int count = countSubGraphs(nodes, p, n, originCell);
    deleteGraph(nodes, p, originCell);
    return count;
}


int main() {
    int counted[30];
    counted[0] = 0;
    for(int i = 1; i < 30; i++) {
        counted[i] = countPolyominoes(i);
        printf("P(%d) = %d\n", i, counted[i] - counted[i - 1]);
    }
    return 0;
}
