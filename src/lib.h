#ifndef OUTPUT_FROM_CPP_H
#define OUTPUT_FROM_CPP_H

#ifdef __cplusplus
extern "C"
{
#endif
    #include <stdint.h>

    extern void myCppFunction(int n);
    void run_bfs_cpp(int64_t root, int64_t* pred);

#ifdef __cplusplus
}
#endif

#endif