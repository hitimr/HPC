#include "bfs.h"
#include <iostream>
#include <vector>
#include <queue>
#include <mpi.h>


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

int g_my_rank;
int g_comm_size;

// Macros copied from reference implementation to handle the graph data structure
// since we cant include c headers from graph500 we need to copy those
#define BYTES_PER_VERTEX 6
#define COLUMN(i) (*(int64_t*)(((char*)column)+(BYTES_PER_VERTEX*i)) & (int64_t)(0xffffffffffffffffULL>>(64-8*BYTES_PER_VERTEX)))


#define MOD_SIZE(v) ((v) & ((1 << lgsize) - 1))
#define DIV_SIZE(v) ((v) >> lgsize)

#define VERTEX_OWNER(v) ((int)(MOD_SIZE(v)))
#define VERTEX_LOCAL(v) ((size_t)(DIV_SIZE(v)))

#define MASTER 0
#define TAG_DEFAULT 0
#define TAG_SOLUTION 1

#define NEXT_VERTEX 0x1


using namespace std;

void run_bfs_cpp(int64_t root, int64_t* pred)
{
    //bfs_serial(root, pred);
    // TODO: bfs_parallel()

    bfs_parallel(root, pred);
    //test_mpi();
}

void bfs_parallel(int64_t root, int64_t* pred)
{
    int64_t v;
    queue<int64_t>* q_work = new queue<int64_t>();    
    queue<int64_t>* q_buffer = new queue<int64_t>();
    queue<int64_t>* tmp; // for swapping

    vector<bool> visited = vector<bool>(g_nglobalverts, false);

    MPI_Comm_rank(MPI_COMM_WORLD, &g_my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &g_comm_size);

    int dest, source;
    if(g_my_rank==0) {dest=1; source=0;}
    else {dest=0; source=1;}

    if(VERTEX_OWNER(root) == g_my_rank)
    {      
		pred[VERTEX_LOCAL(root)] = root;
        visited[root] = true;
        q_work->push(VERTEX_LOCAL(root));
    }

    vector<int64_t> visits;
    while(!q_work->empty())
    {       
        while(!q_work->empty())
        {         
            int64_t u = q_work->front();
            q_work->pop();

            // traverse column of adjecency matrix of vertex u
            for(int64_t j = rowstarts[u]; j < rowstarts[u+1]; j++)
            {
                int64_t v = COLUMN(j);
                if(!visited.at(v)) 
                {
                    visited.at(v) = true;
                    pred_glob[v] = v;
                    visits.push_back(v);
                }
            }
        }

        int64_t visit_size = visits.size();
        vector<int64_t> recv_buffer = vector<int64_t>(g_comm_size);
        MPI_Allgather(
            &visit_size,
            1,
            MPI_INT64_T,
            &recv_buffer[0],
            g_comm_size,
            MPI_INT64_T,
            MPI_COMM_WORLD
        );
        
        MPI_Barrier(MPI_COMM_WORLD);



        for (auto i = recv_buffer.begin(); i != recv_buffer.end(); ++i)
                std::cout << *i << ' ';
        MPI_Barrier(MPI_COMM_WORLD);
    }
    tmp = q_work;
    q_buffer = q_work;
    q_work = tmp;
}

void master(int64_t root, int64_t* pred)
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
    /*

    //cout << "Finished calcs" << endl;
    int dest = 1;
    MPI_Send(
        pred,   //buffer
        g_pred_size, // size
        MPI_INT64_T, // Dtype
        dest, // destination
        TAG_SOLUTION, // Tag
        MPI_COMM_WORLD // Communicator
        );
    */

   int64_t ints[2] = {1,2};
	//MPI_Send(pred,32, MPI_INT64_T,1,0,MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    return;
}

void slave(int64_t root, int64_t* pred)
{
    //int64_t* buffer = (int64_t*)malloc(visited_size * 64 * sizeof(int64_t));
    int tag;
    int my_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /*
    // Wait for new data
    MPI_Recv(
        pred,  // Data Buffer
        g_pred_size,      // length
        MPI_INT64_T, // Dtype
        MASTER, // Source
        TAG_SOLUTION, // Tag
        MPI_COMM_WORLD, // Communicator
        MPI_STATUS_IGNORE
    );  
    cout << "Recieved somethin" << endl;
    */
   
    for(int i = 0; i<32; i++)
        cout << "i=" << i << "\t" << pred_glob[i] << endl;
    MPI_Barrier(MPI_COMM_WORLD);

    return;
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



void test_mpi()
{
    int rank;
	int size;
	int length;
	char name[80];

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Get_processor_name(name, &length);

	int buffer_len = 150;
	char buffer[buffer_len];

	if (rank == 0)
	{
		// Only print from rank 0
        cout << "\n\n---------------------\n";
        cout << "Messages gathered by master:" << endl;
		for (int i = 1; i < size; i++)
		{
			MPI_Recv(buffer, buffer_len, MPI_CHAR, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            cout << buffer << endl;
		}
        cout << "\nEverything recieved" << endl;
        cout << "---------------------\n\n";
	}
	else
	{        
	    sprintf(buffer, "Greetings, master! I am Rank: %d We are %d cores in total. I am running on Machine %s", rank, size, name);
		MPI_Send(buffer, buffer_len, MPI_CHAR, 0, rank, MPI_COMM_WORLD);
	}
}