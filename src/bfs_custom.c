/* Copyright (c) 2011-2017 Graph500 Steering Committee
   All rights reserved.
   Developed by:		Anton Korzh anton@korzh.us
				Graph500 Steering Committee
				http://www.graph500.org
   New code under University of Illinois/NCSA Open Source License
   see license.txt or https://opensource.org/licenses/NCSA
*/

// Graph500: Kernel 2: BFS
// Simple level-synchronized BFS with visits as Active Messages

#include "common.h"
#include "aml.h"
#include "csr_reference.h"
#include "bitmap_reference.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stdint.h>
#include "lib.h"

#ifdef DEBUGSTATS
extern int64_t nbytes_sent,nbytes_rcvd;
#endif
// two arrays holding visited VERTEX_LOCALs for current and next level
// we swap pointers each time
int *q1,*q2;
int qc,q2c; //pointer to first free element

//VISITED bitmap parameters
unsigned long *visited;
int64_t visited_size;

//global variables of CSR graph to be used inside of AM-handlers
int64_t *column;
int64_t *pred_glob;
int * rowstarts;

oned_csr_graph g;

typedef struct visitmsg {
	//both vertexes are VERTEX_LOCAL components as we know src and dest PEs to reconstruct VERTEX_GLOBAL
	int vloc;
	int vfrom;
} visitmsg;

//AM-handler for check&visit
void visithndl(int from,void* data,int sz) {
	visitmsg *m = data;
	if (!TEST_VISITEDLOC(m->vloc)) {
		SET_VISITEDLOC(m->vloc);
		q2[q2c++] = m->vloc;
		pred_glob[m->vloc] = VERTEX_TO_GLOBAL(from,m->vfrom);
	}
}

inline void send_visit(int64_t glob, int from) {
	visitmsg m = {VERTEX_LOCAL(glob),from};
	aml_send(&m,1,sizeof(visitmsg),VERTEX_OWNER(glob));
}

void make_graph_data_structure(const tuple_graph* const tg) {
	int i,j,k;
	convert_graph_to_oned_csr(tg, &g);
	column=g.column;
	rowstarts=g.rowstarts;

	visited_size = (g.nlocalverts + ulong_bits - 1) / ulong_bits;
	aml_register_handler(visithndl,1);
	q1 = xmalloc(g.nlocalverts*sizeof(int)); //100% of vertexes
	q2 = xmalloc(g.nlocalverts*sizeof(int));
	for(i=0;i<g.nlocalverts;i++) q1[i]=0,q2[i]=0; //touch memory
	visited = xmalloc(visited_size*sizeof(unsigned long));
}

void run_bfs_c(int64_t root, int64_t* pred)
{
	int64_t nvisited;
	long sum;
	unsigned int i,j,k,lvl=1;


	CLEAN_VISITED();

	qc=0; sum=1; q2c=0;

	nvisited=1;
	if(VERTEX_OWNER(root) == rank) {
		pred[VERTEX_LOCAL(root)]=root;
		SET_VISITED(root);
		q1[0]=VERTEX_LOCAL(root);
		qc=1;
	} 

	// While there are vertices in current level
	while(sum) 
	{
		//for all vertices in current level send visit AMs to all neighbours
		for(i = 0;i < qc; i++)
			for(j = rowstarts[q1[i]]; j < rowstarts[q1[i]+1]; j++)
				if (!TEST_VISITED(COLUMN(j))) 
				{
					SET_VISITED(COLUMN(j));
					q1[qc++] = COLUMN(j);
					pred_glob[COLUMN(j)] = q1[i];
				}
		sum=qc;
		nvisited+=sum;

		qc=0;
	}
}

void run_bfs(int64_t root, int64_t* pred)
{
	pred_glob=pred;

	//run_bfs_c(root, pred);	
	run_bfs_cpp(root, pred);	

}

//we need edge count to calculate teps. Validation will check if this count is correct
void get_edge_count_for_teps(int64_t* edge_visit_count) {
	long i,j;
	long edge_count=0;
	for(i=0;i<g.nlocalverts;i++)
		if(pred_glob[i]!=-1) {
			for(j=rowstarts[i];j<rowstarts[i+1];j++)
				if(COLUMN(j)<=VERTEX_TO_GLOBAL(my_pe(),i))
					edge_count++;

		}

	aml_long_allsum(&edge_count);
	*edge_visit_count=edge_count;
}

void clean_pred(int64_t* pred) {
	int i;
	for(i=0;i<g.nlocalverts;i++) pred[i]=-1;
}
void free_graph_data_structure(void) {
	int i; 
	free_oned_csr_graph(&g);
	free(q1); free(q2); free(visited);
}

size_t get_nlocalverts_for_pred(void) {
	return g.nlocalverts;
}
