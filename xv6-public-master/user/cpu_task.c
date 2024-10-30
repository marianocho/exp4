#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_VERTICES 200
#define INF 1000000

int graph[MAX_VERTICES][MAX_VERTICES];

void dijkstra(int vertices, int source)
{
    int dist[vertices];
    int visited[vertices];

    for (int i = 0; i < vertices; i++)
    {
        dist[i] = INF;
        visited[i] = 0;
    }
    dist[source] = 0;

    for (int i = 0; i < vertices - 1; i++)
    {
        int min = INF, min_index;

        for (int v = 0; v < vertices; v++)
            if (!visited[v] && dist[v] <= min)
                min = dist[v], min_index = v;

        visited[min_index] = 1;

        for (int v = 0; v < vertices; v++)
            if (!visited[v] && graph[min_index][v] && dist[min_index] != INF &&
                dist[min_index] + graph[min_index][v] < dist[v])
                dist[v] = dist[min_index] + graph[min_index][v];
    }
}

int main(int argc, char *argv[])
{
    int vertices = 100;
    for (int i = 0; i < vertices; i++)
        for (int j = 0; j < vertices; j++)
            graph[i][j] = (i != j) ? (i + j) % 10 + 1 : 0;

    dijkstra(vertices, 0);
    exit();
}
