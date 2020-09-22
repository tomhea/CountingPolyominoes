#include "graphCreator.h"


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


void readGraphFromFile(string path, int* originCell, int*** graph, int* numOfNodes) {
    // Todo
    int** graph = new;;;
    graph = extract(path);
    return graph;
}


// old header:    void deleteGraph(int** nodes, int originCell, int neighboursSize) {
void deleteGraph(int** nodes, u32 numOfNodes) {
//    int* neighboursGlobal = nodes[originCell] - (neighboursSize+1)*originCell;
//    free(nodes);
//    free(neighboursGlobal);

    for (u32 i = 0; i < numOfNodes; i++)
        free(nodes[i]);
    free(nodes);
}

