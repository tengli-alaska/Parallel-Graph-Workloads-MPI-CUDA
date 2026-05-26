// ================= graph_loader.c =================
#include <stdio.h>
#include <stdlib.h>
#include "graph_loader.h"

void load_csr_graph(const char *indptr_file, const char *indices_file, int *num_vertices, int **indptr, int **indices) {
    FILE *f_indptr = fopen(indptr_file, "rb");
    FILE *f_indices = fopen(indices_file, "rb");

    if (!f_indptr || !f_indices) {
        perror("Failed to open CSR files");
        exit(1);
    }

    fseek(f_indptr, 0, SEEK_END);
    long indptr_size = ftell(f_indptr);
    rewind(f_indptr);
    *num_vertices = (indptr_size / sizeof(int)) - 1;

    *indptr = malloc(((*num_vertices) + 1) * sizeof(int));
    fread(*indptr, sizeof(int), (*num_vertices) + 1, f_indptr);

    int num_edges = (*indptr)[*num_vertices];
    *indices = malloc(num_edges * sizeof(int));
    fread(*indices, sizeof(int), num_edges, f_indices);

    fclose(f_indptr);
    fclose(f_indices);
}
