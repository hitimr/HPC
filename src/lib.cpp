#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

using namespace std;

struct oned_csr_graph;

// global variables from bfs_custom.c
extern int64_t *column;
extern int * rowstarts;

void myCppFunction(int n)
{
    std::string bla;
    bla = "BLA BLA";
    cout << bla << endl;
    return;
}

void run_bfs_cpp(int64_t root, int64_t* pred)
{
    cout << column[0] << endl;
    return;
}