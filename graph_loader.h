// ================= graph_loader.h =================
#ifndef GRAPH_LOADER_H
#define GRAPH_LOADER_H

void load_csr_graph(const char *indptr_file, const char *indices_file, int *num_vertices, int **indptr, int **indices);

#endif