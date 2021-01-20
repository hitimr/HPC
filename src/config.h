#define USE_OMP // uncomment if you want to switch off omp usage

#ifdef USE_OMP
    #define LOCAL_THREAD_CNT 2
#endif // use OMP