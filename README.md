#  Hybrid Graph Coloring with MPI, OpenMP, and CUDA
```bash
module load cuda/12.3.0
module load OpenMPI/4.1.6
```

##  Datasets

Place datasets in `CSR` format using two binary files:
- `indptr.bin`: CSR row pointer array
- `indices.bin`: CSR column indices array

Datasets used in this project include:
- `DSJC1000` (DIMACS)
- `Twitter Social Network`
- `New York Road Network`

## Step 1: Preprocess the Graph

Clean the input graphs using the provided `preprocess_csr.py` script. This will:
- Remove self-loops
- Deduplicate edges
- Ensure undirected symmetry

---
### Run the Preprocessing Script

python preprocess_csr.py <indptr.bin> <indices.bin> --out-prefix <prefix>
```bash
python preprocess_csr.py dsjc1000/indptr.bin dsjc1000/indices.bin --out-prefix dsjc1000_clean
python preprocess_csr.py twitter/indptr.bin twitter/indices.bin --out-prefix twitter_clean
python preprocess_csr.py newyork/indptr.bin newyork/indices.bin --out-prefix newyork_clean
```
This will produce:

<prefix>_indptr.bin

<prefix>_indices.bin

These are the cleaned graph files ready for coloring.

---
#### Compile the Hybrid Graph Coloring Project
```bash
make
```
this compiles the following:

main.c (MPI/OpenMP coordination)

graph_loader.c (loads CSR binary files)

graph_coloring.cu (CUDA greedy coloring kernel)

### Run the Graph Coloring Executable

mpirun -n <num_ranks> ./graph_coloring <num_threads> <indptr.bin> <indices.bin>
```bash
mpirun -n 4 ./graph_coloring 8 dsjc1000_clean_indptr.bin dsjc1000_clean_indices.bin
```
This runs the hybrid graph coloring using: 4 MPI processes, 8 OpenMP threads per rank


#  Hybrid Page Rank with MPI, OpenMP, and CUDA

###  Convert Graph to CSR .bin
```bash
make prepare
```
This runs:

python3 prepare_graph.py <INPUT> --prefix <PREFIX>

Example:
```bash
python3 prepare_graph.py amazon0302.txt --prefix amazon
```
### Build PageRank Binary

```bash
make profile    
make debug      
```
### Run PageRank with MPI

```bash
make run
```


