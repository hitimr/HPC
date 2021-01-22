 // uncomment to switch off various optimizations
#define USE_OMP
//#define USE_TESTVISIT_FAST
#define SEND_LOCAL_DATA 

#ifdef USE_OMP
    #define LOCAL_THREAD_CNT 2
#endif // use OMP

#ifdef USE_TESTVISIT_FAST
    #define TEST_VISITED_EMPTY_CUTOFF 0.3
#endif


#define U_SHIFT 32
#define U_MASK 0xffffffff00000000

#define AML_MAX_CHUNK_SIZE 25000
