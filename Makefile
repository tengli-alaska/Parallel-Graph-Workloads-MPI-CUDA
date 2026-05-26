// ================= Makefile =================
CC=mpicc
CXX=nvcc
CFLAGS=-fopenmp -O2
LDFLAGS=-lcudart -L/usr/local/cuda/lib64

all: graph_coloring

graph_coloring: main.o graph_loader.o graph_coloring.o
	$(CC) main.o graph_loader.o -o graph_coloring graph_coloring.o $(LDFLAGS)

main.o: main.c graph_loader.h
	$(CC) $(CFLAGS) -c main.c

graph_loader.o: graph_loader.c graph_loader.h
	$(CC) $(CFLAGS) -c graph_loader.c

graph_coloring.o: graph_coloring.cu
	$(CXX) -c graph_coloring.cu -o graph_coloring.o

clean:
	rm -f *.o graph_coloring vertex_colors.csv
