 // uncomment to switch off various optimizations
#define USE_OMP
//#define USE_TESTVISIT_FAST

#ifdef USE_OMP
    #define LOCAL_THREAD_CNT 2
#endif // use OMP

#ifdef USE_TESTVISIT_FAST
    #define TEST_VISITED_EMPTY_CUTOFF 0.3
#endif

