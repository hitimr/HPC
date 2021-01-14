#ifndef OUTPUT_FROM_CPP_H
#define OUTPUT_FROM_CPP_H

#ifdef __cplusplus
extern "C"
{
#endif
    #include <stdint.h>

    void run_bfs_cpp(int64_t root, int64_t* pred);
    void bfs_serial(int64_t root, int64_t* pred);
    void bfs_parallel(int64_t root, int64_t* pred);

#ifdef __cplusplus
}
#endif

#endif