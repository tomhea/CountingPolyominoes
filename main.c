#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


//typedef struct _list {
//    int value;
//    void* next;
//} * List;

int** createPolyominoGraph(int p) {
    int originCell = p-2;
    int height = 2*p-2;
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

int recCounter(int** nodes, int p, ...) {
    return 0;
}

void deleteGraph(int** nodes, int p) {
    int originCell = p-2;
    int* neighboursGlobal = nodes[originCell] - 5*originCell;
    free(nodes);
    free(neighboursGlobal);
}

int countPolyominoes(int p) {
    int** g = createPolyominoGraph(p);
    int count = deleteGraph(g, p, ...);
    deleteGraph(g, p);
    return count;
}


int main() {
    printf("%d", countPolyominoes(4));
    return 0;
}
