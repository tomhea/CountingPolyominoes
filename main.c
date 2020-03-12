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
int** createPolyominoGraph(int p, int* originCellPtr) {
    int originCell = p>=2 ? p-2 : 0;
    *originCellPtr = originCell;
    int height = p + originCell;
    int n = p*height;

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

int recCounter(int** nodes, int p, int originCell/*, ...*/) {
    return 0;
}

int countSubGraphs(int** nodes, int p, int originCell) {
    return recCounter(nodes, p, originCell/*, ...*/);
}

int countPolyominoes(int p) {
    if (p < 1) return 0;
    int originCell;
    int** nodes = createPolyominoGraph(p, &originCell);
    int count = countSubGraphs(nodes, p, originCell);
    deleteGraph(nodes, p, originCell);
    return count;
}


int main() {
    printf("%d", countPolyominoes(4));
    return 0;
}
