#include "bfs.h"
#include <iostream>
#include <vector>
#include <queue>
#include <mpi.h>

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
    //bfs_serial(root, pred);
    // TODO: bfs_parallel()

    //bfs_parallel(root, pred);
    test_mpi();
}

void bfs_parallel(int64_t root, int64_t* pred)
{
	
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