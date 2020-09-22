#ifndef COUNTINGPOLYOMINOES_GRAPHCREATOR_H
#define COUNTINGPOLYOMINOES_GRAPHCREATOR_H

/*
 * Graphs in this project are represented as list of int* (node),
 *  each node points to an int-array (the node's neighbours).
 *   In the first index there is the number of neighbours, and after it the neighbours themselves (a node is represented as an int).
 */

#include <cstdlib>
#include <string>

typedef unsigned int u32;   // TODO: move whatever we can to unsigned
typedef unsigned long long u64;


using namespace std;

int** createPolyominoGraph(int p, int* originCellPtr, int* nPtr);
int** createPolycubesGraph(int p, int* originCellPtr, int* nPtr);
int** createPolyiamondsGraph(int p, int* originCellPtr, int* nPtr);
void readGraphFromFile(string path, int* originCell, int*** graph, u32* numOfNodes);
void deleteGraph(int** nodes, u32 numOfNodes);

#endif //COUNTINGPOLYOMINOES_GRAPHCREATOR_H
