#ifndef CONFIG_H
#define CONFIG_H

/*
    You can change the cofiguration of the project here. It is recommended to
    make a complete rebuild after making changes.

    Implementations:
        switch bewteen Serial and Parallel implementation. Only one can be active

    Optimizations:
        USE_OMP: adds OMP SIMD to various loops to (hopefully) increase performance
            A slight performance increase has been observed on a desktop machine
            using this feature. 

        USE_TEST_VISIT_FAST: Use the faster version of bit-checking for almost
            empty bitarrays

        DO_NOT_SEND_LOCAL_DATA: saves some performance by not sending local
            data via aml. the effect is bigger on small networks
*/


// --- Implementations
#define BFS_PARALLEL
//#define BFS_SERIAL  // Please make sure thath you call the program with mpirun -np 1

// --- Optimizations
#define USE_OMP
//#define USE_TESTVISIT_FAST
#define DO_NOT_SEND_LOCAL_DATA

// --- Misc
#define TAG_POOLDATA    1
#define AML_MAX_CHUNK_SIZE 4095 // Changing this to  > 4095 require adjustments in aml.h
#define TEST_VISITED_EMPTY_CUTOFF 0.2

// *** do not make changes below this line ***


#define U_SHIFT 32
#define U_MASK 0xffffffff00000000


// Sanity checks
#if defined BFS_PARALLEL && defined BFS_SERIAL
    #error "Please choose either BFS_PARALLEL or BFS_SERIAL not both"
#endif

#if !defined BFS_PARALLEL && !defined BFS_SERIAL
    #error "Please choose either BFS_PARALLEL or BFS_SERIAL"
#endif

#endif // CONFIG_H

