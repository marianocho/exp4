#include "../types.h"
#include "../user.h"
#include "../fcntl.h"
#include "../stat.h"
#include "../fs.h"

#define MAX_VERTICES 200
#define MAX_EDGES 400
#define INF 1000000  // Representa o infinito para distâncias inacessíveis

// Estrutura para o grafo
int graph[MAX_VERTICES][MAX_VERTICES];
int dist[MAX_VERTICES];  // Distâncias mínimas
int visited[MAX_VERTICES];  // Marca vértices visitados

// Implementação simples de função rand para XV6
unsigned int seed = 12345;
int rand() {
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

void initialize_random_graph(int vertices, int edges) {
    // Garantir que vertices e edges não excedam os limites
    if (vertices > MAX_VERTICES) {
        vertices = MAX_VERTICES; // Limitar ao máximo permitido
    }
    
    if (edges > MAX_EDGES) {
        edges = MAX_EDGES; // Limitar ao máximo permitido
    }

    // Inicializar todas as arestas como "infinito"
    for (int i = 0; i < vertices; i++) {
        for (int j = 0; j < vertices; j++) {
            if (i != j) {
                graph[i][j] = INF;
            } else {
                graph[i][j] = 0;
            }
        }
    }

    // Adicionar arestas aleatórias com pesos entre 1 e 20
    for (int i = 0; i < edges; i++) {
        int u = rand() % vertices;
        int v = rand() % vertices;
        int weight = 1 + rand() % 20;
        if (u != v) {
            graph[u][v] = weight;
        }
    }
}

// Função para encontrar o vértice com a menor distância
int min_distance(int vertices) {
    int min = INF, min_index = -1;
    for (int v = 0; v < vertices; v++) {
        if (!visited[v] && dist[v] <= min) {
            min = dist[v];
            min_index = v;
        }
    }
    return min_index;
}

// Implementação do algoritmo de Dijkstra
void dijkstra(int src, int vertices) {
    for (int i = 0; i < vertices; i++) {
        dist[i] = INF;
        visited[i] = 0;
    }
    dist[src] = 0;

    for (int count = 0; count < vertices - 1; count++) {
        int u = min_distance(vertices);
        visited[u] = 1;

        for (int v = 0; v < vertices; v++) {
            if (!visited[v] && graph[u][v] != INF && dist[u] != INF &&
                dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
            }
        }
    }
}

// Função para gerar grafos aleatórios e calcular caminho mínimo
void generate_and_solve_graph() {
    int vertices = 100 + (rand() % 100);  // Entre 100 e 200 vértices
    int edges = 50 + (rand() % 350);      // Entre 50 e 400 arestas

    initialize_random_graph(vertices, edges);

    // Escolha um vértice inicial aleatório
    int src = rand() % vertices;
    dijkstra(src, vertices);
}

int main() {
    for (int i = 0; i < 1000; i++) {
        generate_and_solve_graph();
    }
    exit();
}
