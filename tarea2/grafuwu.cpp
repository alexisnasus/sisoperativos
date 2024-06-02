#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <set>
#include <utility>
#include <thread>
#include <vector>
#include <semaphore.h>
#include <map>
#include <chrono>

#define NODES 20 // nodos
#define N_THREADS 20
#define START_NODE 0
#define END_NODE (NODES - 1)
#define DURATION 60 // tiempo en segundos
#define M 2 // Máximo número de threads que pueden transitar por una arista al mismo tiempo
#define INT_MAX 2147483647 // Define INT_MAX si no se incluye <climits>

using namespace std;

struct Node {
    int dest;
    int cost;
    struct Node *next;
};

struct List {
    struct Node *head;
};

struct Graph {
    struct List *arr;
};

struct Node *newNode(int dest, int cost) {
    struct Node *node = (struct Node *)malloc(sizeof(struct Node));
    node->dest = dest;
    node->cost = cost;
    node->next = NULL;
    return node;
}

struct Graph *createGraph(int V) {
    struct Graph *graph = (struct Graph *)malloc(sizeof(struct Graph));
    graph->arr = (struct List *)malloc(V * sizeof(struct List));
    for (int i = 0; i < V; ++i)
        graph->arr[i].head = NULL;
    return graph;
}

void addEdge(struct Graph *graph, int src, int dest, int cost, map<pair<int, int>, sem_t>& semaphores) {
    struct Node *node = newNode(dest, cost);
    node->next = graph->arr[src].head;
    graph->arr[src].head = node;

    // Inicializar el semáforo para esta arista
    sem_t sem;
    sem_init(&sem, 0, M);
    semaphores[make_pair(src, dest)] = sem;
}

void printGraph(struct Graph *graph, int V) {
    for (int i = 0; i < V; ++i) {
        struct Node *tmp = graph->arr[i].head;
        printf("\nNodos adyacentes al nodo %d\n", i);
        set<int> seen;
        while (tmp) {
            if (seen.find(tmp->dest) == seen.end()) {
                printf(" -> %d (costo: %d) \n", tmp->dest, tmp->cost);
                seen.insert(tmp->dest);
            }
            tmp = tmp->next;
        }
        printf("\n");
    }
}

sem_t globalSem; // Semáforo para proteger la actualización de la mejor ruta

struct ThreadBest {
    int cost;
    vector<int> path;
};

ThreadBest threadBests[N_THREADS]; // Almacenar el mejor costo y ruta de cada hilo

void findRoutes(Graph* graph, map<pair<int, int>, sem_t>& semaphores, int thread_id) {
    auto end_time = chrono::steady_clock::now() + chrono::seconds(DURATION);
    while (chrono::steady_clock::now() < end_time) {
        int currentCost = 0;
        vector<int> currentPath;
        int currentNodeIndex = START_NODE;
        currentPath.push_back(currentNodeIndex);
        Node* currentNode = graph->arr[currentNodeIndex].head;

        while (currentNode != NULL) {
            // Esperar en el semáforo para esta arista
            sem_t &edgeSem = semaphores[make_pair(currentNodeIndex, currentNode->dest)];
            sem_wait(&edgeSem);

            currentCost += currentNode->cost;
            currentNodeIndex = currentNode->dest;
            currentPath.push_back(currentNodeIndex);

            if (currentNode->dest == END_NODE) {
                sem_wait(&globalSem); // Esperar (bloquear) el semáforo global
                if (currentCost < threadBests[thread_id].cost) {
                    threadBests[thread_id].cost = currentCost;
                    threadBests[thread_id].path = currentPath;
                    printf("Thread %d encontró una nueva mejor ruta con costo: %d\n", thread_id, currentCost);
                }
                sem_post(&globalSem); // Señalar (desbloquear) el semáforo global
                sem_post(&edgeSem); // Liberar el semáforo para esta arista
                break;
            }

            // Esperar un segundo antes de decidir aleatoriamente el siguiente nodo
            this_thread::sleep_for(chrono::seconds(1));

            // Decidir aleatoriamente el siguiente nodo
            vector<Node*> nextNodes;
            Node* temp = graph->arr[currentNode->dest].head;
            while (temp) {
                nextNodes.push_back(temp);
                temp = temp->next;
            }
            if (nextNodes.empty()) {
                sem_post(&edgeSem); // Liberar el semáforo si no hay más nodos
                break;
            }
            currentNode = nextNodes[rand() % nextNodes.size()];
            sem_post(&edgeSem); // Liberar el semáforo para esta arista antes de moverse al siguiente nodo
        }
    }
}

int main() {
    srand(time(0)); // semilla para la aleatoriedad
    struct Graph *graph = createGraph(NODES);
    map<pair<int, int>, sem_t> semaphores; // Mapa de semáforos para cada arista

    // Inicializar el semáforo global con valor 1 (binario)
    sem_init(&globalSem, 0, 1);

    // Inicializar los mejores costos para cada hilo
    for (int i = 0; i < N_THREADS; ++i) {
        threadBests[i].cost = INT_MAX;
    }

    // Generar aristas aleatorias sin ciclos
    for (int i = 0; i < NODES; ++i) { // recorrer los nodos
        int numEdges = 5 + rand() % 7; // número aleatorio de aristas para cada nodo
        set<int> existingEdges; // para asegurar que cada destino sea único
        for (int j = 0; j < numEdges; ++j) { // agregar aristas aleatoriamente a los nodos
            if (i < NODES - 1) { // asegúrese de que no intentemos generar un destino ilegal
                int dst = i + 1 + (rand() % (NODES - i - 1));
                if (dst < NODES && existingEdges.find(dst) == existingEdges.end()) {
                    int cost = rand() % (NODES * 2); // costo aleatorio
                    existingEdges.insert(dst);
                    addEdge(graph, i, dst, cost, semaphores);
                }
            }
        }
    }

    printGraph(graph, NODES);

    thread threads[N_THREADS];
    for (int i = 0; i < N_THREADS; i++) {
        threads[i] = thread(findRoutes, graph, ref(semaphores), i);
    }

    for (int i = 0; i < N_THREADS; i++) {
        threads[i].join();
    }

    // Destruir los semáforos
    for (auto &entry : semaphores) {
        sem_destroy(&entry.second);
    }
    sem_destroy(&globalSem);

    // Encontrar el mejor costo entre todos los hilos
    int finalBestCost = INT_MAX;
    int bestThreadID = -1;
    for (int i = 0; i < N_THREADS; ++i) {
        if (threadBests[i].cost < finalBestCost) {
            finalBestCost = threadBests[i].cost;
            bestThreadID = i;
        }
    }

    printf("Mejor costo final: %d\n", finalBestCost);
    printf("Encontrado por el thread: %d\n", bestThreadID);
    printf("Mejor ruta final: ");
    for (int i = 0; i < threadBests[bestThreadID].path.size(); i++) {
        if (i != 0) printf(" -> ");
        printf("%d", threadBests[bestThreadID].path[i]);
    }
    printf(" -> FIN\n");

    return 0;
}

