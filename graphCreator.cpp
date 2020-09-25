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

    int** nodes = (int**)malloc(n * sizeof(int*));
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


/// \param p - max number of movement in the graph, from the origin cell.
/// \param originCellPtr - output ptr for the origin cell of the graph.
/// \param nPtr - output ptr for the number of vertices in the graph.
/// \return - the created graph.
int** createPolyominoGraphImproved(int p, int* originCellPtr, u32* nPtr) {
    int originCell = p>=2 ? p-2 : 0;
    *originCellPtr = originCell;
    int height = p + originCell;
    int n = p*height;
    *nPtr = n;

    int** nodes = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        nodes[i] = (int*)malloc((1+4)*sizeof(int));
    }

    for(int i = originCell; i < n; i++) {   // TODO: delete the four inner 'else' if want normal (but slower) graph
        int* neighbours = nodes[i];
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
    }
    return nodes;
}


void readGraphFromFile(const char* path, int* originCell, int*** graph, u32* numOfNodes) {
    // Todo
//    printf("%s\n", path);
    *graph = createPolyominoGraphImproved(atoi(path), originCell, numOfNodes);
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











/////
///// \param nodes - the graph itself.
///// \param p - size of max sub-graphs.
///// \param n - number of vertices in graph.
///// \param originCell - starting node for any sub-graph.
///// \return - number of sub-graphs of 'nodes' contains at most 'p' nodes, including 'originCell'.
//u64 countSubGraphs(int** nodes, int p, int n, int originCell, const char* backupName) {
//    if (p <= 0) return 0;
//
//    bool* nodesFound = (bool*)calloc(n, sizeof(bool));
//    int* untriedSet = (int*)malloc(n * sizeof(int));
//    nodesFound[originCell] = true;
//    untriedSet[0] = originCell;
//
//    char path[80], tempPath[80];
//    char p_str[5];
//    itoa(p, p_str, 10);
//    strcpy(path, backupName);
//    strcat(path, "_");
//    if (p < 10) strcat(path, "0");
//    strcat(path, p_str);
//    strcat(path, ".txt");
//    strcpy(tempPath, "_temp_");
//    strcat(tempPath, path);
//
//    int** oldUntriedSetEndStack = (int**)malloc(p * sizeof(int*));
//    int** untriedSetStack = (int**)malloc(p * sizeof(int*));
//
//
//    u64 count = recJobsCreatorGOTO(nodes, nodesFound, untriedSet, untriedSet+1, oldUntriedSetEndStack, untriedSetStack, p, n, path, tempPath);
//    free(oldUntriedSetEndStack);
//    free(untriedSetStack);
//
//    free(nodesFound);
//    free(untriedSet);
//    return count;
//}

/////
///// \param p - max size of polyominoes.
///// \return - counts the number of fixed polyominoes up to the size of p.
//u64 countPolyominoes(int p) {
//    if (p < 1) return 0;
//    int originCell, n;
//    int** nodes = createPolyominoGraph(p, &originCell, &n);
//    u64 count = countSubGraphs(nodes, p, n, originCell, "Polyomino");
//    deleteGraph(nodes, originCell, 4);
//    return count;
//}
//
//
/////
///// \param p - max size of polycubes.
///// \return - counts the number of fixed polycubes up to the size of p.
//u64 countPolycubes(int p) {
//    if (p < 1) return 0;
//    int originCell, n;
//    int** nodes = createPolycubesGraph(p, &originCell, &n);
//    u64 count = countSubGraphs(nodes, p, n, originCell, "Polycube");
//    deleteGraph(nodes, originCell, 6);
//    return count;
//}
//
///
/// \param p - max size of polyiamonds.
/// \return - counts the number of fixed polyiamonds up to the size of p,
///             including those start with left -pointing-triangle (count1),
///                   and those start with right-pointing-triangle (count2 - originCell-1)
//u64 countPolyiamonds(int p) {
//    if (p < 1) return 0;
//    int originCell, n;
//    int** nodes = createPolyiamondsGraph(p, &originCell, &n);
//    u64 count = countSubGraphs(nodes, p, n, originCell, "Polyiamond");
//    deleteGraph(nodes, originCell, 3);
//    return count;

//    u64 count1 = countSubGraphs(nodes, p  , n, originCell, "Polyiamond_count1");
//    u64 count2 = countSubGraphs(nodes, p-1, n, originCell, "Polyiamond_count2");
//    deleteGraph(nodes, originCell, 3);
//    return count1 + count2;
//}
