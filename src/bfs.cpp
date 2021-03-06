#include "bfs.h"
#include <iostream>
#include <string.h>
#include <vector>
#include <list>
#include <queue>
#include <limits.h>
#include <mpi.h>
#include <algorithm>
#include "../aml/aml.h"
#include "config.h"
#include "pool.h"

#ifdef USE_OMP
#include <omp.h>
#endif


struct oned_csr_graph;

// global variables from bfs_custom.c
extern int64_t *column;
extern int * rowstarts;
extern int64_t visited_size;   
extern int64_t *pred_glob;
extern int lgsize;

// custom globals
extern int64_t g_nlocalverts;
extern int64_t g_nglobalverts;

unsigned long *visited;

int g_my_rank;  // TODO remove before release
int g_comm_size;

int my_rank;
int comm_size;

// Macros copied from reference implementation to handle the graph data structure
// since we cant include c headers from graph500 we need to copy those
#define BYTES_PER_VERTEX 6
#define COLUMN(i) (*(int64_t*)(((char*)column)+(BYTES_PER_VERTEX*i)) & (int64_t)(0xffffffffffffffffULL>>(64-8*BYTES_PER_VERTEX)))


#define MOD_SIZE(v) ((v) & ((1 << lgsize) - 1))
#define DIV_SIZE(v) ((v) >> lgsize)
#define MUL_SIZE(x) ((x) << lgsize)

#define VERTEX_OWNER(v) ((int)(MOD_SIZE(v)))
#define VERTEX_LOCAL(v) ((size_t)(DIV_SIZE(v)))
#define VERTEX_TO_GLOBAL(r, i) ((int64_t)(MUL_SIZE((uint64_t)((i))) + (int)((r))))

#define ulong_bits 64
#define ulong_mask &63
#define ulong_shift >>6
#define SET_VISITED(v) do {visited[VERTEX_LOCAL((v)) ulong_shift] |= (1UL << (VERTEX_LOCAL((v)) ulong_mask));} while (0)
#define SET_VISITEDLOC(v) do {visited[(v) ulong_shift] |= (1ULL << ((v) ulong_mask));} while (0)
#define TEST_VISITED(v) ((visited[VERTEX_LOCAL((v)) ulong_shift] & (1UL << (VERTEX_LOCAL((v)) ulong_mask))) != 0)
#define TEST_VISITEDLOC(v) ((visited[(v) ulong_shift] & (1ULL << ((v) ulong_mask))) != 0)
#define CLEAN_VISITED()  memset(visited,0,visited_size*sizeof(unsigned long));

using namespace std;

#ifdef USE_TESTVISIT_FAST
inline bool test_visited_empty(int64_t v) { return visited[(v) ulong_shift] ? TEST_VISITEDLOC(v) : false; }
inline bool test_visited_mixed(int64_t v) { return TEST_VISITEDLOC(v); }
inline bool test_visited_full(int64_t v) { return visited[(v) ulong_shift] == 0xffffffffffffffff ? true : TEST_VISITEDLOC(v); }
bool (*test_visited_fast)(int64_t);

#endif // USE_TESTVISIT_FAST


queue<int64_t>* q_work;    
queue<int64_t>* q_buffer;

inline void send_pool(vector<int64_t> & pool, int dest, int64_t u) 
{    
    int64_t chunk_start = 0;
    int64_t chunk_len;
    int64_t vertices_remaining = pool.size();

    // Since AML has a hardcoded limit on its bufffer size we need to split up
    // the data so it does not exeed this limit
    while(vertices_remaining)
    {
        if(vertices_remaining <= AML_MAX_CHUNK_SIZE)
        {
            // number of vertices is smaller that the maximum chunk size
            chunk_len = vertices_remaining;  
            vertices_remaining = 0;   
        } 
        else
        {
            // remaining number of vertices is bigger than ther maximum chunk size
            // we can only send a part of the data
            chunk_len = AML_MAX_CHUNK_SIZE;
            vertices_remaining -= chunk_len;
        }
        
        // AML works best if we can send a sequential line of data. Unfortunately
        // we need to send the predecessor vertex from the queue as well with 
        // every call of aml_send().Since we only need SCALE bits to address every
        // vertex and SCALE <32 for the forseeable future we can just encode it 
        // into the first vertex' 8 most significat bytes
        pool[chunk_start] |= (u << U_SHIFT); 
        aml_send(&pool[chunk_start], TAG_POOLDATA, sizeof(int64_t)*chunk_len, dest);
        chunk_start += chunk_len;
    }
}

void analyze_pool(int from, void* data, int sz) 
{
	int64_t* pool_data = static_cast<int64_t*>(data);   // cast data to proper type
    int size = sz/sizeof(int64_t);  // calculate transmission size
    
    int64_t u = (pool_data[0] >> U_SHIFT);  // Extract u from first entry
    pool_data[0] &= ~(U_MASK);  // clear out u with a mask

    // Check visited array and add unvisited vertices to buffer
    #pragma omp simd
    for(int i = 0; i < size; i++)
    {       
        int64_t vertex = VERTEX_LOCAL(pool_data[i]);

        #ifdef USE_TESTVISIT_FAST
            if (!(*test_visited_fast)(vertex))    // "faster" bit test function
        #else
            if(!TEST_VISITEDLOC(vertex))  // reference macro
        #endif                         
        {
            SET_VISITEDLOC(vertex);
            q_buffer->push(vertex);
            pred_glob[vertex] = VERTEX_TO_GLOBAL(from, u);
        }
    }
}

void run_bfs_cpp(int64_t root, int64_t* pred)
{
    pred_glob=pred;

    #ifdef BFS_PARALLEL
        bfs_parallel(root, pred);
    #else
        bfs_serial(root, pred);
    #endif
    
}

void bfs_parallel(int64_t root, int64_t* pred)
{
    int64_t queue_size, n_local_visits;

    q_work =    new queue<int64_t>();    
    q_buffer =  new queue<int64_t>();
    CLEAN_VISITED() // Set everything in the bit array to 0

    #ifdef USE_TESTVISIT_FAST
        test_visited_fast= &test_visited_empty;
    #endif
    
	aml_register_handler(analyze_pool, TAG_POOLDATA);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    vector<vector<int64_t>> pool(comm_size);   
    for(int i = 0; i < comm_size; i++)
        pool[i].reserve(g_nlocalverts); // reserve space for the vector so it does not "grow"

    // add root to the queue of the responsible rank
	if(VERTEX_OWNER(root) == my_rank) 
    {
		pred[VERTEX_LOCAL(root)] = root;
		SET_VISITED(root);
		q_work->push(VERTEX_LOCAL(root));
	} 

    // BFS algorithm
    n_local_visits = 0;
    do // while there are vertices left on any queue
    {       
        while(!q_work->empty())
        {         
            int64_t u = q_work->front();
            q_work->pop();

            // traverse column of adjecency matrix of vertex u
            #pragma omp simd
            for(int64_t j = rowstarts[u]; j < rowstarts[u+1]; j++)
            {
                int64_t vertex = COLUMN(j); // Extract vertex from CSR row
                int owner = VERTEX_OWNER(vertex);   // calculate the responsible thread of the vertex
                pool[owner].push_back(vertex);  // add it to the respective pool
            }

            // work through pool
            for(int i=0; i < pool.size(); i++)
            {
                #ifdef DO_NOT_SEND_LOCAL_DATA
                    if(i != my_rank)    // Only senmd non-local vertices
                #else                    
                    if(true)    // Always send local vertices via AML/MPI
                #endif
                {        
                    if(pool[i].size() > 0)  // only send if there is something in the pool
                        send_pool(pool[i], i, u);
                } 
                else 
                {
                    for(auto itr = pool[i].begin(); itr != pool[i].end(); itr++)
                    {  
                        int64_t vloc = VERTEX_LOCAL(*itr);  
                         
                        #ifdef USE_TESTVISIT_FAST
                            if (!(*test_visited_fast)(vloc))    // "faster" bit test function
                        #else
                            if(!TEST_VISITEDLOC(vloc))  // reference macro
                        #endif                         
                        {                                           
                            SET_VISITEDLOC(vloc);
                            q_buffer->push(vloc);
                            pred_glob[vloc] = VERTEX_TO_GLOBAL(my_rank, u);
                        }
                    }

                }  
                #ifdef USE_TESTVISIT_FAST
                    n_local_visits += pool[i].size();
                    // switch to regular test_visit function once the visited bit-array has more than TEST_VISITED_EMPTY_CUTOFF high bits
                    if((n_local_visits / (float) g_nlocalverts) > TEST_VISITED_EMPTY_CUTOFF) 
                        test_visited_fast = &test_visited_mixed;
                #endif   

                pool[i].clear();
                pool[i].reserve(g_nlocalverts); // manually set the growth strategy of the vector
            }
        }
        aml_barrier();

        swap(q_work, q_buffer);

        // check how many elemtens are left in the working queue globally
        queue_size = (int64_t) q_work->size();  
        MPI_Allreduce(MPI_IN_PLACE, &queue_size, 1, MPI_INT64_T, MPI_SUM,MPI_COMM_WORLD);
    }
    while(queue_size);  // reserve space for the vector so it does not "grow"
    aml_barrier();
}


// Serial BFS-Algorithm taken from 
// https://www.research.manchester.ac.uk/portal/files/60828897/FULL_TEXT.PDF page 34 Algorithm 1
void bfs_serial(int64_t root, int64_t* pred)
{
    int64_t u,j,v;

    // Initializing the visited array. the predecessor list is cleared before urn_bfs() is called
    vector<bool> vis(visited_size*64+1, false);
    queue<int64_t> q; // Create empty queue



    q.push(root); // Enter the starting vertex into the queue
    vis[root] = true;
    pred[root] = root;

    while(!q.empty())
    {
        u = q.front();
        q.pop();

        for(j = rowstarts[u]; j < rowstarts[u+1]; j++)
        {
            v = COLUMN(j);
            if(!vis[v]) 
            {
                vis[v] = true;
                q.push(v);
                pred_glob[v] = u;
            }
        }
    }
    return;
}
