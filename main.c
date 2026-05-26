// ================= main.c =================
#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph_loader.h"

extern void cuda_graph_coloring(int *indptr, int *indices, int *colors, int start_vertex, int end_vertex, int num_vertices);
void save_colors_to_csv(const char *filename, int *colors, int num_vertices);
void resolve_conflicts(int *colors, int *indptr, int *indices, int start_vertex, int end_vertex);

double timestamp(const char *label, double t_start, int rank) {
    double t_end = MPI_Wtime();
    if (rank == 0) {
        printf("[TIMER] %s took %.6f seconds\n", label, t_end - t_start);
    }
    return t_end;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: %s <num_threads> <indptr_file> <indices_file>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    omp_set_num_threads(num_threads);

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const char *indptr_file = argv[2];
    const char *indices_file = argv[3];

    int num_vertices;
    int *indptr = NULL, *indices = NULL;

    double t0_total = MPI_Wtime();
    double t0 = MPI_Wtime();

    if (rank == 0) {
        load_csr_graph(indptr_file, indices_file, &num_vertices, &indptr, &indices);
    }
    t0 = timestamp("Graph Load", t0, rank);

    MPI_Bcast(&num_vertices, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank != 0) {
        indptr = malloc((num_vertices + 1) * sizeof(int));
    }
    MPI_Bcast(indptr, num_vertices + 1, MPI_INT, 0, MPI_COMM_WORLD);

    int num_edges = indptr[num_vertices];
    if (rank != 0) {
        indices = malloc(num_edges * sizeof(int));
    }
    MPI_Bcast(indices, num_edges, MPI_INT, 0, MPI_COMM_WORLD);
    t0 = timestamp("MPI Broadcast", t0, rank);

    int vtx_per_rank = (num_vertices + size - 1) / size;
    int start_vertex = rank * vtx_per_rank;
    int end_vertex = (rank + 1) * vtx_per_rank;
    if (end_vertex > num_vertices) end_vertex = num_vertices;

    int *colors = malloc(num_vertices * sizeof(int));
    for (int i = 0; i < num_vertices; i++) colors[i] = -1;

    t0 = MPI_Wtime();
    cuda_graph_coloring(indptr, indices, colors, start_vertex, end_vertex, num_vertices);
    t0 = timestamp("CUDA Greedy Coloring", t0, rank);

    t0 = MPI_Wtime();
    resolve_conflicts(colors, indptr, indices, start_vertex, end_vertex);
    t0 = timestamp("OpenMP Conflict Resolution", t0, rank);

    MPI_Barrier(MPI_COMM_WORLD);
    t0 = MPI_Wtime();

    int *global_colors = NULL;
    if (rank == 0) global_colors = malloc(num_vertices * sizeof(int));
    MPI_Reduce(colors, global_colors, num_vertices, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
    t0 = timestamp("MPI Reduce + Output Prep", t0, rank);

    double t1_total = MPI_Wtime();

    if (rank == 0) {
        int max_color = 0;
        for (int i = 0; i < num_vertices; i++) {
            if (global_colors[i] > max_color) max_color = global_colors[i];
        }
        printf("\n[RESULT] Minimum number of colors used: %d\n", max_color + 1);
        printf("[TOTAL TIME] Execution Time: %.6f seconds\n", t1_total - t0_total);

        save_colors_to_csv("vertex_colors.csv", global_colors, num_vertices);
        free(global_colors);
    }

    free(indptr);
    free(indices);
    free(colors);
    MPI_Finalize();
    return 0;
}

void save_colors_to_csv(const char *filename, int *colors, int num_vertices) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to write CSV");
        return;
    }
    fprintf(fp, "Vertex,Color\n");
    for (int i = 0; i < num_vertices; i++) {
        fprintf(fp, "%d,%d\n", i, colors[i]);
    }
    fclose(fp);
}

void resolve_conflicts(int *colors, int *indptr, int *indices, int start_vertex, int end_vertex) {
    #pragma omp parallel for
    for (int v = start_vertex; v < end_vertex; v++) {
        for (int i = indptr[v]; i < indptr[v + 1]; i++) {
            int u = indices[i];
            if (u >= start_vertex && u < end_vertex && u < v && colors[u] == colors[v]) {
                colors[v]++;
            }
        }
    }
}
