// ================= graph_coloring.cu =================
#include <cuda_runtime.h>
#include <stdio.h>

__global__ void color_kernel(int *indptr, int *indices, int *colors, int start_v, int end_v, int num_vertices) {
    int v = blockIdx.x * blockDim.x + threadIdx.x + start_v;
    if (v >= end_v) return;

    extern __shared__ int forbidden[];

    for (int i = threadIdx.x; i < num_vertices; i += blockDim.x) forbidden[i] = 0;
    __syncthreads();

    for (int i = indptr[v]; i < indptr[v+1]; i++) {
        int u = indices[i];
        int c = colors[u];
        if (c >= 0) forbidden[c] = 1;
    }
    __syncthreads();

    for (int c = 0; c < num_vertices; c++) {
        if (!forbidden[c]) {
            colors[v] = c;
            break;
        }
    }
}

extern "C" void cuda_graph_coloring(int *indptr, int *indices, int *colors, int start_vertex, int end_vertex, int num_vertices) {
    int *d_indptr, *d_indices, *d_colors;
    int num_edges = indptr[num_vertices];

    cudaMalloc(&d_indptr, (num_vertices + 1) * sizeof(int));
    cudaMalloc(&d_indices, num_edges * sizeof(int));
    cudaMalloc(&d_colors, num_vertices * sizeof(int));

    cudaMemcpy(d_indptr, indptr, (num_vertices + 1) * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_indices, indices, num_edges * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_colors, colors, num_vertices * sizeof(int), cudaMemcpyHostToDevice);

    int blockSize = 256;
    int numThreads = end_vertex - start_vertex;
    int numBlocks = (numThreads + blockSize - 1) / blockSize;

    color_kernel<<<numBlocks, blockSize, num_vertices * sizeof(int)>>>(d_indptr, d_indices, d_colors, start_vertex, end_vertex, num_vertices);

    cudaMemcpy(colors, d_colors, num_vertices * sizeof(int), cudaMemcpyDeviceToHost);

    cudaFree(d_indptr);
    cudaFree(d_indices);
    cudaFree(d_colors);
}