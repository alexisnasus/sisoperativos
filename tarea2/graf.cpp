#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <set>
#include <utility>
#include <semaphore.h>
#include <thread>
#include <stack>

#define NODES 20 // nodos
#define M_CONCURRENCY 3
#define N_THREADS 20


using namespace std;

struct Node {
    int dest;
    int cost;
    struct Node *next;
};

struct Route {
    int suma = n->cost;
    stack<Node> passed; 
    struct Node *n;
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
void addEdge(struct Graph *graph, int src, int dest, int cost) {
    struct Node *node = newNode(dest, cost);
    node->next = graph->arr[src].head;
    graph->arr[src].head = node;
}
void printGraph(struct Graph *graph, int V) {
    for (int i = 0; i < V; ++i) {
        struct Node *tmp = graph->arr[i].head;
        printf("\nNodos adyacentes al nodo %d\n", i);
        while (tmp) {
            printf(" -> %d (costo: %d) \n", tmp->dest, tmp->cost);
            tmp = tmp->next;
        }
        printf("\n");
    }
}

sem_t sem[NODES];
struct Route hilo[N_THREADS];
int sem_limit = M_CONCURRENCY;
int cont = 0;


void findRoute(sem_t &sem){

    sem_wait(&sem);

    sem_limit--;
    printf("Thread %d compró un ticket. Tickets disponibles: %d\n", 1, sem_limit);

    sem_post(&sem);
}

int main() {
    srand(time(0)); // semilla para la aleatoriedad
    struct Graph *graph = createGraph(NODES);

    // Generar aristas aleatorias sin ciclos
    for (int i = 0; i < NODES; ++i) { // recorrer los nodos
        int numEdges = 5 + rand() % 7; // número aleatorio de aristas para cada nodo
        set<pair<int, int>> existingEdges; // para asegurar costos únicos por arista
        for (int j = 0; j < numEdges; ++j) { // agregar aristas aleatoriamente a los nodos
            if (i < NODES - 1) { // asegúrese de que no intentemos generar un destino ilegal
                int dst = i + 1 + (rand() % (NODES - i - 1));
                if (dst < NODES) {
                    int cost = rand() % (NODES * 2); // costo aleatorio
                    while (existingEdges.find(make_pair(dst, cost)) != existingEdges.end()) {
                        cost = rand() % (NODES * 2); // generar un nuevo costo si ya existe
                    }
                    existingEdges.insert(make_pair(dst, cost));
                    addEdge(graph, i, dst, cost);
                }
            }
        }
    }

    printGraph(graph, NODES);
    return 0;
}
