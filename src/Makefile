CFLAGS = -Drestrict=__restrict__ -O1 -w -DGRAPH_GENERATOR_MPI -DREUSE_CSR_FOR_VALIDATION -I../aml -g
LDFLAGS = -lpthread
MPICC = mpicc
MPICXX = mpic++

all: graph500_reference_bfs test bfs_custom

GENERATOR_SOURCES = ../generator/graph_generator.c ../generator/make_graph.c ../generator/splittable_mrg.c ../generator/utils.c
SOURCES = main.c utils.c validate.c ../aml/aml.c custom.c
HEADERS = common.h csr_reference.h bitmap_reference.h custom.h lib.h

lib: lib.cpp lib.h
	$(MPICXX) -std=c++11 -c -g lib.cpp

graph500_reference_bfs: bfs_reference.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES) csr_reference.c
	$(MPICC) $(CFLAGS) $(LDFLAGS) -o graph500_reference_bfs bfs_reference.c csr_reference.c $(SOURCES) $(GENERATOR_SOURCES) -lm

bfs_custom: bfs_custom.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES) csr_reference.c lib.o 
	$(MPICC) $(CFLAGS) $(LDFLAGS) -o graph500_custom_bfs bfs_custom.c csr_reference.c $(SOURCES) $(GENERATOR_SOURCES) lib.o -lstdc++ -lm
	./graph500_custom_bfs 18 | grep median_time

test: custom.c custom_test.c
	gcc -g -o custom_test custom.c custom_test.c
	./custom_test

clean:
	-rm -f graph500_*
	-rm -f custom_test
	-rm -f *.o
