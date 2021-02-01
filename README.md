# Breadth-first search (BFS) project for High Performance Computing for WS2020

## System Requirements

* C/C++ compiler supporting C++11 or later
* MPI implementation for C and C++

## Compiling the project

To install and build the project:

    git clone https://github.com/hitimr/HPC
    cd HPC
    make

please note that the makefile is in the root dir and note in ./src

Check if the project works (launch with 2 processes and scale = 18):

    make run


Manually launch the binary:

    cd src/build
    ./bfs [SCALE] [EDGEFACTOR]


Clean environment

    make clean


When compiling on the hydra cluster please make sure that the correct MPI module is loaded as some do not support C++.
We tested all our code with openmpi 3.1.3

    module load mpi/openmpi-3.1.3-slurm



## Original Readme by graph 500


Graph500-3.0.0

Compiling should be pretty straightforward as long as you have a valid MPI-3 library loaded in your PATH.
There is no more OpenMP,Sequential and XMT versions of benchmark.

On single node you can run MPI code with reasonable performance.

To build binaries change directory to src and execute make.
If you are lucky four binaries would be built, two of which are of interest:

graph500_reference_bfs runs BFS kernel (and skips weight generation)
graph500_reference_bfs_sssp runs both BFS and SSSP kernels

Both binaries require one integer parameter which is scale of the graph.
Validation can be deactivated by specifying SKIP_VALIDATION=1 as an environment variable.
bfs_sssp binary would skip BFS part if SKIP_BFS=1 is present in your environment.

If you want to store/read generated graph from/to file use environment variables TMPFILE=<filename> and also REUSEFILE=1 to keep the file.
It's advised to use bfs_sssp binary to generate graph files as it generates both files of edges and weights (filename.weights)
bfs binary would only use/write edges file. And once bfs_sssp cant open weights file it would generate both files even if edges files is present.

N.B:

Current settings assume you are using powers of 2: total number of cores and number of cores per node.
It's possible to have non-power of two of nodes if you comment macro defined in common.h SIZE_MUST_BE_POWER_OF_TWO.
Be aware normally that will drop your performance by more then 20%.

If you want to use non-power of two processes per node, you should add -DPROCS_PER_NODE_NOT_POWER_OF_TWO to CFLAGS in src/Makefile,
this one will enable SIZE_MUST_BE_POWER_OF_TWO automatically.
