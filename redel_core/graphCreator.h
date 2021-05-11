#ifndef COUNTINGPOLYOMINOES_GRAPHCREATOR_H
#define COUNTINGPOLYOMINOES_GRAPHCREATOR_H

#include "defs.h"


/*
 * Graphs in this project are represented as list of int* (node),
 *  each node points to an int-array (the node's neighbours).
 *   In the first index there is the number of neighbours, and after it the neighbours themselves (a node is represented as an int).
 */


//int** createPolyominoGraph(int p, int* originCellPtr, int* nPtr);
//int** createPolycubesGraph(int p, int* originCellPtr, int* nPtr);
//int** createPolyiamondsGraph(int p, int* originCellPtr, int* nPtr);
void createPolyominoGraphFile(const char* path, int size);
void readGraphFromFile(const char* path, int* originCell, int*** graph, u32* numOfNodes);
void deleteGraph(int** graph, u32 numOfNodes);

#endif //COUNTINGPOLYOMINOES_GRAPHCREATOR_H
