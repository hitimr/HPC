// uncomment to switch off various optimizations
#define USE_OMP
//#define USE_TESTVISIT_FAST
//#define SEND_LOCAL_DATA 

#ifdef USE_TESTVISIT_FAST
    #define TEST_VISITED_EMPTY_CUTOFF 0.2
#endif


#define U_SHIFT 32
#define U_MASK 0xffffffff00000000


#define TAG_POOLDATA    1
#define AML_MAX_CHUNK_SIZE 4095

