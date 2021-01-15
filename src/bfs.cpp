#include "bfs.h"
#include <vector>
#include <queue>


using namespace std;

struct oned_csr_graph;

// global variables from bfs_custom.c
extern int64_t *column;
extern int * rowstarts;
extern int64_t visited_size;
extern int64_t *pred_glob;

// required for COLUMN()-Macro
#define BYTES_PER_VERTEX 6
#define COLUMN(i) (*(int64_t*)(((char*)column)+(BYTES_PER_VERTEX*i)) & (int64_t)(0xffffffffffffffffULL>>(64-8*BYTES_PER_VERTEX)))


void run_bfs_cpp(int64_t root, int64_t* pred)
{
    bfs_serial(root, pred);
    // TODO: bfs_parallel()

    //bfs_parallel(root, pred);
}


// Serial BFS-Algorithm taken from 
// https://www.research.manchester.ac.uk/portal/files/60828897/FULL_TEXT.PDF page 34 Algorithm 1
void bfs_serial(int64_t root, int64_t* pred)
{
    return;
}