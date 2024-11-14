#include "../types.h"
#include "../user.h"
#include "../fcntl.h"
#include "../stat.h"

#define MAX_VERTICES 200
#define MAX_EDGES 400
#define INF 1000000  // Represents infinity for unreachable distances

// Function to initialize a random graph
void initialize_random_graph(int **graph, int vertices, int edges) {
    for (int i = 0; i < vertices; i++) {
        for (int j = 0; j < vertices; j++) {
            if (i != j) {
                graph[i][j] = INF;
            } else {
                graph[i][j] = 0;
            }
        }
    }

    for (int i = 0; i < edges; i++) {
        int u = custom_rand() % vertices;
        int v = custom_rand() % vertices;
        int weight = 1 + custom_rand() % 20;
        if (u != v) {
            graph[u][v] = weight;
        }
    }
}

// Function to find the vertex with the minimum distance
int min_distance(int *dist, int *visited, int vertices) {
    int min = INF, min_index = -1;
    for (int v = 0; v < vertices; v++) {
        if (!visited[v] && dist[v] <= min) {
            min = dist[v];
            min_index = v;
        }
    }
    return min_index;
}

// Dijkstra's algorithm implementation
void dijkstra(int **graph, int *dist, int *visited, int src, int vertices) {
    for (int i = 0; i < vertices; i++) {
        dist[i] = INF;
        visited[i] = 0;
    }
    dist[src] = 0;

    for (int count = 0; count < vertices - 1; count++) {
        int u = min_distance(dist, visited, vertices);
        visited[u] = 1;

        for (int v = 0; v < vertices; v++) {
            if (!visited[v] && graph[u][v] != INF && dist[u] != INF &&
                dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
            }
        }
    }
}

// Function to generate random graphs and solve them
void generate_and_solve_graph(unsigned int *total_alloc_time, unsigned int *total_free_time) {
    int vertices = 100 + (custom_rand() % 100);  // Between 100 and 200 vertices
    int edges = 50 + (custom_rand() % 350);      // Between 50 and 400 edges

    int start_time, end_time;

    // Dynamically allocate the graph
    start_time = uptime();
    int **graph = malloc(vertices * sizeof(int *));
    for (int i = 0; i < vertices; i++) {
        graph[i] = malloc(vertices * sizeof(int));
    }
    int *dist = malloc(vertices * sizeof(int));
    int *visited = malloc(vertices * sizeof(int));
    end_time = uptime();
    *total_alloc_time += (end_time - start_time);

    if (graph == 0 || dist == 0 || visited == 0) {
        printf(1, "[CPU] Error allocating memory\n");
        return;
    }

    // Initialize the graph and run Dijkstra
    initialize_random_graph(graph, vertices, edges);
    int src = custom_rand() % vertices;
    dijkstra(graph, dist, visited, src, vertices);

    // Dynamically free memory
    start_time = uptime();
    for (int i = 0; i < vertices; i++) {
        free(graph[i]);
    }
    free(graph);
    free(dist);
    free(visited);
    end_time = uptime();
    *total_free_time += (end_time - start_time);
}

int main() {
    custom_srand(12345); // Set the seed

    unsigned int total_alloc_time = 0;
    unsigned int total_free_time = 0;

    for (int i = 0; i < 1000; i++) {
        generate_and_solve_graph(&total_alloc_time, &total_free_time);
    }

    // Write the times to the stats file
    char stats_filename[32];
    int pid = getpid();
    build_filename(stats_filename, pid, "stats_mem_");

    // Write the allocation and deallocation times to the stats file
    int fd = open(stats_filename, O_CREATE | O_WRONLY);
    if (fd >= 0) {
        char buffer[100];
        int n = 0;
        n += int_to_str(total_alloc_time, buffer + n);
        buffer[n++] = ' ';
        n += int_to_str(total_free_time, buffer + n);
        buffer[n++] = '\n';
        write(fd, buffer, n);
        close(fd);
    } else {
        printf(1, "[CPU] Error writing to stats file\n");
    }

    exit();
}
