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
extern int64_t g_pred_size; // TODO: remove before release
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
#endif // USE_TESTVISIT_FAST

// TODO: maybe switch to enums
#define TAG_HEADER      2
#define TAG_POOLDATA    3

queue<int64_t>* q_work;    
queue<int64_t>* q_buffer;

MPI_Status status;


typedef struct visitmsg {
	//both vertexes are VERTEX_LOCAL components as we know src and dest PEs to reconstruct VERTEX_GLOBAL
	int vloc;
	int vfrom;
} visitmsg;

typedef struct header_t {
    size_t size;
    int source;
} header_t;


//AM-handler for check&visit
void visithndl(int from, void* data, int sz) 
{
	int64_t* pool_data = static_cast<int64_t*>(data);
    int size = sz/sizeof(int64_t);

    for(int i = 0; i < (size - 1); i++)
    {       
        int64_t vertex = VERTEX_LOCAL(pool_data[i]);

        if (!TEST_VISITEDLOC(vertex)) 
        {
            SET_VISITEDLOC(vertex);
            q_buffer->push(vertex);
            pred_glob[vertex] = VERTEX_TO_GLOBAL(from, pool_data[size-1]);
        }
    }
}

void process_pool_hndl(int from, void* data, int sz)
{
    header_t* h = (header_t*) data;    

    vector<int64_t> rx_data = vector<int64_t>(h->size);    // Allocate ressources for pool contents
    
    
    MPI_Recv(
        &rx_data[0],
        h->size,
        MPI_INT64_T,
        h->source,
        TAG_POOLDATA,
        MPI_COMM_WORLD,
        &status
    );

    
    cout << "Recieved " << rx_data.size() << endl;
    for(int i = 0; i < h->size; i++)
    {        
        if (!TEST_VISITEDLOC(rx_data[i])) 
        {
            //SET_VISITEDLOC(rx_data[i]);
            //q_buffer->push(rx_data[i]);
            //pred_glob[rx_data[i]] = VERTEX_TO_GLOBAL(from, rx_data[i]);
        }
    }
    
    //delete(buffer);
}

inline void send_visit(vector<int64_t> & pool, int dest, int64_t u) 
{
    pool.push_back(u);   // hack: just append the u so we can send it as well. TODO: replace with proper message
	aml_send(&pool[0], 1, sizeof(int64_t)*pool.size(), dest);
    pool.pop_back();
}

inline void send_pool(vector<int64_t> & pool, int dest)
{
    // Send header containing information about the pool
    header_t msg = { pool.size(), my_rank };
    aml_send(&msg, TAG_HEADER, sizeof(msg), dest);

    // Send actual data
    MPI_Send(
        &pool[0],
        pool.size(),
        MPI_INT64_T,
        dest,
        TAG_POOLDATA,
        MPI_COMM_WORLD
    );
}



void run_bfs_cpp(int64_t root, int64_t* pred)
{
    pred_glob=pred;
    //bfs_serial(root, pred);

    bfs_parallel(root, pred);
}

void bfs_parallel(int64_t root, int64_t* pred)
{
#ifdef USE_OMP
    omp_set_num_threads(LOCAL_THREAD_CNT);
#endif

    int64_t v;
    int64_t queue_size, n_local_visits;

    q_work =    new queue<int64_t>();    
    q_buffer =  new queue<int64_t>();

    
	aml_register_handler(visithndl,1);  // TODO: remove before release
    //aml_register_handler(process_pool_hndl,2);
    CLEAN_VISITED()
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    vector<vector<int64_t>> pool(comm_size);

#ifdef USE_TESTVISIT_FAST
    bool (*test_visited_fast)(int64_t) = &test_visited_full;
#endif

	if(VERTEX_OWNER(root) == my_rank) 
    {
		pred[VERTEX_LOCAL(root)] = root;
		SET_VISITED(root);
		q_work->push(VERTEX_LOCAL(root));
	} 

    n_local_visits = 0;
    do
    {       
        while(!q_work->empty())
        {         
            int64_t u = q_work->front();
            q_work->pop();

            // traverse column of adjecency matrix of vertex u
            for(int64_t j = rowstarts[u]; j < rowstarts[u+1]; j++)
            {
                int64_t vertex = COLUMN(j);
                int owner = VERTEX_OWNER(vertex);
                pool[owner].push_back(vertex);
            }

            // work through pool
            for(int i=0; i < pool.size(); i++)
            {
                #ifdef SEND_LOCAL_DATA
                    if(true)    // Always send local verices via AML/MPI
                #else
                    if(i != my_rank)    // Only senmd non-local vertices
                #endif
                {        
                    if(pool[i].size() > 0)  // only send if there is something in the pool
                        send_visit(pool[i], i, u);
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
                    #ifdef USE_TESTVISIT_FAST
                        n_local_visits += pool[i].size();
                        // switch to regular test_visit function once the visited bit-array has more than TEST_VISITED_EMPTY_CUTOFF high bits
                        if((n_local_visits / (float) g_nlocalverts) > TEST_VISITED_EMPTY_CUTOFF) 
                            test_visited_fast = &test_visited_mixed;
                    #endif
                }               

                pool[i].clear();
            }
        }
        aml_barrier();

        swap(q_work, q_buffer);

        // check how many elemtens are left in the working queue globally
        queue_size = (int64_t) q_work->size();  
        MPI_Allreduce(MPI_IN_PLACE, &queue_size, 1, MPI_INT64_T, MPI_SUM,MPI_COMM_WORLD);
    }
    while(queue_size); // repeat as long as there are queue elements left globally
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
