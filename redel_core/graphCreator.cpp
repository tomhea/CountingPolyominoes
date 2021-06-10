#include "graphCreator.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>

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

    for(int i = 0; i < originCell; i++){
        nodes[i][0] = 0;
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


/// 
/// \param path
/// \param size
void createPolyominoGraphFile(const char* path, int size){
    ofstream file;
    file.open (path);
    file << "4.4.4.4 " << size << endl;
    int originCell;
    u32 numOfNodes;
    int** graph = createPolyominoGraphImproved(size, &originCell, &numOfNodes);
    file << numOfNodes << endl;
    file << originCell << endl;
//    file << 0 << endl;
    for (u32 i = 0; i < numOfNodes; ++i) {
        file << i << " " << graph[i][0];
        for (int j = 0; j < graph[i][0]; ++j) {
            file << " " << graph[i][1+j];
        }
        file << endl;
    }
    file.close();
}

int** extractGraph(int** nodesData, int* originCell, u32* numOfNodes){
    int** graph = (int**)malloc(*numOfNodes * sizeof(int*));
    map <int, int> idConverter;
    int newIds = 0;

    int id;
    int numOfNeighbors;
    int* neighbors;

    for (u32 i = 0; i < *numOfNodes; i++) {
        id = nodesData[i][0];
        if(idConverter.find(id) == idConverter.end())
            idConverter.insert(pair<int,int>(id,newIds++));
    }
    for (u32 i = 0; i < *numOfNodes; i++) {
        id = nodesData[i][0];

        numOfNeighbors = nodesData[i][1];
        neighbors = (int*) malloc((1 + numOfNeighbors) * sizeof(int));
        neighbors[0] = numOfNeighbors;
        for (int j = 0; j < numOfNeighbors; ++j) {
            // if(idConverter.find(nodesData[i][2+j]) == idConverter.end())
            //     idConverter.insert(pair<int,int>(nodesData[i][2+j],newIds++));
            neighbors[j+1] = idConverter.at(nodesData[i][2+j]);
        }

        graph[idConverter.at(id)] = neighbors;
    }
    *originCell = idConverter.at(*originCell);
    return graph;
}


void readGraphFromFile(const char* path, int* originCell, int*** graph, u32* numOfNodes) {
    ifstream file(path);
    string number;
    stringstream ss;

//    int protocol;

    if (file.is_open())
    {
        string graphMetadata;
        string graphData;
        getline(file, graphMetadata);
        getline(file, graphData);
        *numOfNodes = (u32)stoi(graphData);
        getline(file, graphData);
        *originCell = stoi(graphData);
//        getline(file, graphData);
//        ss << graphData;
//        ss >> number;
//        protocol = stoi(number);
//        switch(protocol) {
//            case REGULAR_GRAPH:
//                break;
//            case MIN_MAX_CONDITION_GRAPH:
//                ss >> number;
//                int min = stoi(number);
//                ss >> number;
//                int max = stoi(number);
//                //TODO pass it forward;
//                break;
//        }
        int** nodesData = (int**)malloc(*numOfNodes * sizeof(int*));
        
        string line;
        int id, numOfNeighbors;
        for (u32 i = 0; i < *numOfNodes; ++i){
            stringstream ss;
            getline(file, line);
            ss << line;

            ss >> number;
            id = stoi(number);
            ss >> number;
            numOfNeighbors = stoi(number);
            nodesData[i] = (int*)malloc((2 + numOfNeighbors) * sizeof(int));
            nodesData[i][0] = id;
            nodesData[i][1] = numOfNeighbors;
            for (int j = 0; j < numOfNeighbors; ++j) {
                ss >> number;
                nodesData[i][j+2] = stoi(number);
            }
        }
        int** nodes = extractGraph(nodesData, originCell, numOfNodes);
        *graph = nodes;
        for (u32 i = 0; i < *numOfNodes; ++i)
            free(nodesData[i]);
        free(nodesData);
    } else {
        printf("File %s didn't load\n", path);
    }
    file.close();
}

void deleteGraph(int** graph, u32 numOfNodes) {
    for (u32 i = 0; i < numOfNodes; i++)
        free(graph[i]);
    free(graph);
}
