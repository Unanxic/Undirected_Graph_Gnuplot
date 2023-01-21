//Mpatila Dimitra 4127
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <math.h>

#define MAX_NUM_OF_NODES 100
#define MIN_ALLOWED_PROXIMITY 5.0
#define ready 1
#define waiting 2
#define processed 3
#define TRUE 1
#define FALSE 0


struct node {
    int nodeLabel;
    struct node* next;
};

struct Graph {
    int numOfNodes;
    int* headPointers; // The actual nodeLabel number, e.g. for node 1 it will be 0
    struct node** adjLists;
};

struct cartesianPoint {
    float x;
    float y;
};


void createAdjacencyMatrix();
int isConnected();
void breadthFirstSearch(int);
void insertNodeToQueue(int);
int deleteFromQueue();
int isQueueEmpty();
int getRandomColor();
struct Graph * calculateAdjacencyLists();
void extractNodesAndEdgesInfoToDatFile();
void searchNode();
void addNode();
void deleteNode();
int nodeExists(int);
void plotGraph();
int getNumOfLines(FILE *fp);
struct node* createNode(int label);
void layoutGraph(int numOfNodes, struct cartesianPoint *coordinatesList, int max, int min);
void printAdjacencyLists(struct Graph *graph);
void printAdjacencyMatrix();

int adjacencyMatrix[MAX_NUM_OF_NODES][MAX_NUM_OF_NODES];
static int currentNumberOfNodes;
int state[MAX_NUM_OF_NODES]; //[INITIAL, WAITING, VISITED]
int queue[MAX_NUM_OF_NODES], front = -1,rear = -1;
FILE *fptr;

int main() {
    int connected = 0, wantsToExit = 0;
    char choice;
    //Create the graph E(G) and V(G) by asking user input, only escape while loop if the resulting graph is connected
    while (connected == 0) {
        createAdjacencyMatrix();
        connected = isConnected();
    }

    // Calculate and Print AdjLists
    struct Graph *graph = calculateAdjacencyLists();
    printAdjacencyLists(graph);
    extractNodesAndEdgesInfoToDatFile();

    // User menu
    do {
        printf("-------------------------\n");
        printf("Search a node - Press S\n");
        printf("Insert a node - Press I\n");
        printf("Delete a node - Press D\n");
        printf("Print graph - Press P\n");
        printf("Exit - Press any other key\n");
        printf("Your option: ");
        getchar(); //Consume newline character
        struct Graph *graph;
        scanf("%c", &choice);
        if (choice == 'S' || choice == 's') {
            searchNode();
        }
        else if (choice == 'I' || choice == 'i') {
            addNode();
            graph = calculateAdjacencyLists();
            printAdjacencyLists(graph);
            extractNodesAndEdgesInfoToDatFile();
        }
        else if (choice == 'D' || choice == 'd') {
            deleteNode();
            graph = calculateAdjacencyLists();
            printAdjacencyLists(graph);
            extractNodesAndEdgesInfoToDatFile();
        }
        else if (choice == 'P' || choice == 'p') {
            // Extract to dat file and print graph
            plotGraph();
        }
        else {
            wantsToExit = 1;
        }
    } while (wantsToExit == 0);
    printf("Exiting...\n");
    return 0;
}

/**
 * Get the numbers of lines in FILE fp
 * @param fp
 * @return
 */
int getNumOfLines(FILE *fp) {
    char ch;
    int lines = 0;
    while (!feof(fp)) {
        ch = fgetc(fp);
        if (ch == '\n') {
            lines++;
        }
    }
    return lines;
}

void plotGraph() {
    FILE *edgesFile, *nodesFile, *formattedDataFile;
    formattedDataFile = fopen("graph3.dat", "w");
    edgesFile = fopen("edges.dat", "r");
    nodesFile = fopen("vertices.dat", "r");

    if(edgesFile == NULL || formattedDataFile == NULL || nodesFile == NULL) {exit(1);}

    char line[500];
    char **lines;
    int numOfNodes = getNumOfLines(nodesFile); //Each node occupies 1 line in vertices.dat

    lines = (char**) (malloc((numOfNodes + 1) * sizeof(char *)));
    if (lines == NULL) {exit(1);}
    for (int i = 0; i < numOfNodes + 1; i++) {
        *(lines + i) = (char *) malloc(sizeof(char *) * 40);
        if (lines[i] == NULL) { exit(1); }
    }
    //nodes
    struct cartesianPoint* coordinates_list;
    coordinates_list = (struct  cartesianPoint *) malloc(sizeof(struct  cartesianPoint) * numOfNodes);
    if (coordinates_list == NULL) { exit(1);};
    int allocations_cnt = 0; //counts how many coordinates have been allocated
    int max = 11, min = 1;

    layoutGraph(numOfNodes, coordinates_list, max, min); //Assign coordinates

    fprintf(formattedDataFile, "\n\n"); //Gnuplot seems to need these two empty lines to plot the graph correctly
    strcpy(lines[0], "# ID  x     y     PointColor  Name\n");
    fprintf(formattedDataFile, lines[0]);
    int cnt = 0;
    fseek(nodesFile, 0, SEEK_SET); //Set the file pointer to index 0, so we don't skip a line by accident
    while (fgets(line, sizeof(line) - 2, nodesFile)) {
        int color_rgb = getRandomColor();
        line[strlen(line)-1] = '\0';
        fprintf(formattedDataFile, "  %-4s%-6.1f%-6.1f%-8d%-4s\n", line, coordinates_list[cnt].x, coordinates_list[cnt].y, color_rgb, line);
        cnt++;
    }
    printf("\n");


    for (int i = 0; i < numOfNodes + 1; i++) {
        free(lines[i]);
    }
    free(lines);

    int numOfEdges = getNumOfLines(edgesFile);
    lines = (char **)(malloc((numOfEdges) * sizeof(char *)));
    for (int i = 0; i < numOfEdges; i++)
    {
        lines[i] = (char *)malloc(sizeof(char *) * 25);
        if (lines[i] == NULL)
        {
            exit(1);
        }
    }
    fprintf(formattedDataFile, "\n\n");
    fprintf(formattedDataFile, "# ID1 ID2   LineColor\n");

    fseek(edgesFile, 0, SEEK_SET);
    while (fgets(line, sizeof(line) - 2, edgesFile)) {
        int color_rgb = getRandomColor();
        line[strlen(line)-1] = '\0';
        fprintf(formattedDataFile, "%s %d\n", line, color_rgb);
    }

    for (int i = 0; i < numOfEdges; i++)
    {
        free(lines[i]);
    }
    free(lines);

    fclose(nodesFile);
    fclose(edgesFile);
    fclose(formattedDataFile);
    system("gnuplot -p  < gnuplot_script.gp");
}

void layoutGraph(int numOfNodes, struct cartesianPoint *coordinatesList, int max, int min) {
    for (int ci = 0; ci < numOfNodes; ci++) { //decide layout using linear/cartesian distance
        int acceptablePlacementFound = 0; int max_allowed_attempts = 10;
        for (int attempt = 0; attempt < max_allowed_attempts && acceptablePlacementFound == 0; attempt++) {
            coordinatesList[ci].x = ((max - min) * ((float)rand() / (float)RAND_MAX)) + min;
            coordinatesList[ci].y = ((250 - min) * ((float)rand() / (float)RAND_MAX)) + min;
            for (int j = 0; j < ci; j++) {
                float distanceX = coordinatesList[j].x - coordinatesList[ci].x;
                float distanceY = coordinatesList[j].y - coordinatesList[ci].y;
                float distance = sqrt((distanceX * distanceX) + (distanceY * distanceY));
                if (distance < MIN_ALLOWED_PROXIMITY) {
                    acceptablePlacementFound = 0;
                    break;
                } else if (j == ci - 1) { //We found an acceptable placement when all prior nodes checked are not too close
                    acceptablePlacementFound = 1;
                }
            }
        }
    }
}

/**
 * Get random color
 * @return A value between 10 and 240 [RGB- > 0 - 255]
*/
int getRandomColor() {
    int result = (rand() % 230 + 10); //between 10 and 240 RGB
    return result;
}

/**
 * Calculate the adjacency lists
 */
struct Graph * calculateAdjacencyLists() {

    //A graph
    struct Graph* graph = (struct Graph*)malloc(sizeof(struct Graph));
    if (graph == NULL) {exit(1);}
    //With adjacent lists
    graph->adjLists = (struct node *) malloc(currentNumberOfNodes * sizeof(struct node *));
    if (graph->adjLists == NULL) { exit(1);}
    //and a list for the nodes pointing to their corresponding adjacent list
    //because adjacency lists is a graph representation as an array (headPointers) of
    //linked lists (adjLists)
    graph->headPointers = (int *)malloc(currentNumberOfNodes * sizeof(int));
    if (graph->headPointers == NULL) { exit(1);}

    int countLists = 0;
    for(int i = 0; i < MAX_NUM_OF_NODES; i++) {
        if (adjacencyMatrix[i][i] == 2 || adjacencyMatrix[i][i] == 1) {
            graph->adjLists[countLists] = NULL;
            graph->headPointers[countLists] = i;
            countLists++;
        }
    }
    graph->numOfNodes = countLists;

    // Populate Lists by adding nodes in the beggining of the corresponding adjacency list
    for(int i = 0; i < countLists; i++) {
        for(int j = 0; j < MAX_NUM_OF_NODES; j++) {
            if (adjacencyMatrix[graph->headPointers[i]][j] == 1 ) {
                struct node* newNode = createNode(j);
                newNode->next = graph->adjLists[i];
                graph->adjLists[i] = newNode;
            }
        }
    }
    return graph;
}

/**
 * Print adjacency lists
 * @param graph the undirected graph
 */
void printAdjacencyLists(struct Graph *graph) {
    printf("--- Printing Adjacent lists ---\n");
    for (int v = 0; v < graph->numOfNodes; v++) {
        struct node* temp = graph->adjLists[v];
        if (temp == NULL) { exit(1);}
        printf("\n Node %d -> ", graph->headPointers[v]);
        while (temp) {
            printf("%d -> ", temp->nodeLabel);
            temp = temp->next;
        }
        printf("\n");
    }
    printf("------------------------------------\n");
}

/**
 * Create a new node
 * @param label the node label
 * @return
 */
struct node* createNode(int label) {
    struct node* newNode = (struct node*) malloc(sizeof(struct node)); //Allocate memory for new struct node
    if (newNode == NULL) { exit(1);}
    newNode->nodeLabel = label;
    newNode->next = NULL;
    return newNode;
}

void createAdjacencyMatrix() {
    int i, j, maxEdges, origin, destination;
    printf("\nEnter number of nodes : ");
    scanf("%d", &currentNumberOfNodes);
    maxEdges = currentNumberOfNodes * (currentNumberOfNodes - 1) / 2 + currentNumberOfNodes; //+currentNumberOfNodes for edges that are from the node to the node itself
    for(i = 0; i < MAX_NUM_OF_NODES; i++) {
        for(j = 0; j < MAX_NUM_OF_NODES; j++) {
            if (i == j && i < currentNumberOfNodes) {
                adjacencyMatrix[i][j] = 2;
            }
            else {
                adjacencyMatrix[i][j] = 0;
            }
        }
    }

    for(i = 0; i < maxEdges; i++) {
        printf("\nEnter edge %d [Quit: -1 -1]: ", i);
        scanf("%d%d", &origin, &destination);
        if ((origin == -1) && (destination == -1)) {
            break;
        }
        if(origin >= currentNumberOfNodes || destination >= currentNumberOfNodes || origin < 0 || destination < 0) {
            printf("\nInvalid edge!\n");
            i--;
        }
        else {
            adjacencyMatrix[origin][destination] = 1;
            adjacencyMatrix[destination][origin] = 1;
        }
    }
}

int isConnected() {
    int node;
    int connected = 1;
    //Set all nodes to state::ready, so they are ready to explore
    for (node = 0; node < currentNumberOfNodes; node++) {
        state[node] = ready;
    }
    breadthFirstSearch(0); //breadthFirstSearch from node 0
    for(node = 0; node < currentNumberOfNodes; node++) {
        if(state[node] == ready) { //If any node has still not been visited then the graph is not connected
            connected = 0;
            break;
        }
    }
    if(connected) {
        printf("\nGraph is connected\n");
        return TRUE;
    }
    else {
        printf("\nGraph is not connected\n");
        return FALSE;
    }
}

void breadthFirstSearch(int node) {
    insertNodeToQueue(node);
    state[node] = waiting;
    while(!isQueueEmpty() ) {
        node = deleteFromQueue();
        state[node] = processed;
        for(int i=0; i <= currentNumberOfNodes - 1; i++) {
            //Check for adjacent unvisited nodes
            if(adjacencyMatrix[node][i] == 1 && state[i] == ready) {
                insertNodeToQueue(i);
                state[i] = waiting;
            }
        }
    }
}

/**
 * Insert node to queue
 * @param node the node's label
 */
void insertNodeToQueue(int node) {
    if(rear == MAX_NUM_OF_NODES - 1)
        printf("\nQueue is empty\n");
    else {
        if(front == -1) {
            front = 0;
        }
        rear = rear+1;
        queue[rear] = node ;
    }
}

/**
 * Check if queue is empty
 * @return int 1 if empty
 */
int isQueueEmpty() {
    if(front == -1 || front > rear )
        return 1;
    else
        return 0;
}

/**
 * Delete node from queue
 * @return the deleted node
 */
int deleteFromQueue() {
    if (front == -1 || front > rear) {
        printf("\nQueue is empty\n");
        exit(1);
    }
    int nodeToDeleted = queue[front];
    front = front+1;
    return nodeToDeleted;
}

void extractNodesAndEdgesInfoToDatFile() {
    int i, j;
    fptr = fopen("vertices.dat", "w");
    for(i = 0; i < MAX_NUM_OF_NODES; i++) {
        if (adjacencyMatrix[i][i] == 2 || adjacencyMatrix[i][i] == 1) {
            fprintf(fptr, "%d\n", i + 1);
        }
    }
    fclose(fptr);
    fptr = fopen("edges.dat", "w");
    for(i = 0; i < MAX_NUM_OF_NODES; i++) {
        for(j = i; j < MAX_NUM_OF_NODES; j++) {
            if (adjacencyMatrix[i][j] == 1) {
                fprintf(fptr, "%d %d\n", i + 1, j + 1);
            }
        }
    }
    fclose(fptr);
}

/**
 * Search a node by label/name
 */
void searchNode() {
    int node;
    printf("\nNode name: ");
    scanf("%d", &node);
    if (nodeExists(node) == 1) {
        printf("\nNode exists\n");
    }
    else {
        printf("\nNode doesn't exist\n");
    }
}


/**
 * Add node
 */
void addNode() {
    int node, i, destination;
    printf("\nNode label: ");
    scanf("%d", &node);
    if (nodeExists(node) == 1) {
        printf("\nThe node already exists\n");
    }
    else { //If it doesnt exist
        adjacencyMatrix[node][node] = 2; //Add node to the adjacency matrix
        for(i = 0; i < MAX_NUM_OF_NODES - 1; i++) {
            printf("\nProvide name of node to connect to [-1 if no more nodes]: ");
            scanf("%d", &destination);
            if(destination == -1) { //Exit for loop
                break;
            }
            //If the user provided bad input, decrement i and let them try again
            if(destination >= MAX_NUM_OF_NODES || destination < 0
               || nodeExists(destination) == 0) {
                printf("\nInvalid edge!\n");
                i--;
            }
            else if (adjacencyMatrix[node][destination] == 1) { //Edge already exists
                printf("\nEdge already in graph\n");
                i--;
            }
            else { //Add the edge to the adjacency matrix
                adjacencyMatrix[node][destination] = 1;
                adjacencyMatrix[destination][node] = 1;
            }
        }
        currentNumberOfNodes++;
        printf("\nNode added\n");
    }
}

/**
 * Delete a node by setting its
 * adjacency matrix locations to 0
 */
void deleteNode() {
    int node, i;
    printf("\nEnter node to delete : ");
    scanf("%d", &node);
    if (nodeExists(node) == 0) {
        printf("\nThe node does not exist\n");
    }
    else {
        for (i = 0; i < MAX_NUM_OF_NODES; i++) { //Remove all references of nodeX from the matrix
            adjacencyMatrix[node][i] = 0;
            adjacencyMatrix[i][node] = 0;
        }
        currentNumberOfNodes--;
        printf("\nDeleted\n");
    }
}

/**
 * Check if the node exists
 */
int nodeExists(int node) {
    int nodeInGraph = 0;
    if (adjacencyMatrix[node][node] == 1 || adjacencyMatrix[node][node] == 2) {
        nodeInGraph = 1;
    }
    return nodeInGraph;
}

void printAdjacencyMatrix() {
    for (int i = 0; i < MAX_NUM_OF_NODES; i++) {
        for (int j = 0; j < MAX_NUM_OF_NODES; j++) {
            printf("%d ", adjacencyMatrix[i][j]);
        }
        printf("\n");
    }
}