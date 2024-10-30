#include "../types.h"
#include "../user.h"
#include "../fcntl.h"
#include "../stat.h"
#include "../fs.h"

#define MAX_VERTICES 200
#define MAX_EDGES 400
#define INF 1000000

int graph[MAX_VERTICES][MAX_VERTICES];
int dist[MAX_VERTICES];
int visited[MAX_VERTICES];

unsigned int seed = 12345;
int rand() {
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

void initialize_random_graph(int vertices, int edges) {
    for (int i = 0; i < vertices; i++) {
        for (int j = 0; j < vertices; j++) {
            graph[i][j] = (i != j) ? INF : 0;
        }
    }

    for (int i = 0; i < edges; i++) {
        int u = rand() % vertices;
        int v = rand() % vertices;
        int weight = 1 + rand() % 20;
        if (u != v) {
            graph[u][v] = weight;
            graph[v][u] = weight;
        }
    }
}

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

void generate_and_solve_graph() {
    int vertices = 100 + (rand() % 100);
    int edges = 50 + (rand() % 350);

    initialize_random_graph(vertices, edges);
    int src = rand() % vertices;
    dijkstra(src, vertices);
}

int main() {
    uint start_time = uptime();

    for (int i = 0; i < 1000; i++) {
        generate_and_solve_graph();
    }

    uint end_time = uptime();
    int elapsed = end_time - start_time;
    printf(1, "CPU-bound tempo total: %d ticks\n", elapsed);
    exit();
}
