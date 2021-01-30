// switch between implementations
#define BFS_PARALLEL
//#define BFS_SERIAL  // Please make sure thath you call the program with mpirun -np 1

// uncomment to switch off various optimizations
#define USE_OMP
//#define USE_TESTVISIT_FAST
//#define SEND_LOCAL_DATA // TODO chenge dont send local data


#ifdef USE_TESTVISIT_FAST
    #define TEST_VISITED_EMPTY_CUTOFF 0.2
#endif


#define U_SHIFT 32
#define U_MASK 0xffffffff00000000


#define TAG_POOLDATA    1
#define AML_MAX_CHUNK_SIZE 4095





// Sanity checks
#if defined BFS_PARALLEL && defined BFS_SERIAL
    #error "Please choose either BFS_PARALLEL or BFS_SERIAL not both"
#endif

#if !defined BFS_PARALLEL && !defined BFS_SERIAL
    #error "Please choose either BFS_PARALLEL or BFS_SERIAL"
#endif

