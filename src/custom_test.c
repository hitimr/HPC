#include "custom.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define TEST_OUT stderr

int test_bitmap();
int test_queue();



int main()
{
   test_bitmap();
   test_queue();
}

int test_queue()
{
    queue_t* q = queue_new();
    
    int test_len = 100;
    int test_vals[test_len];
    int i;
    for(i=0; i < test_len; i++)
        test_vals[i] = i;

    for(i=0; i < test_len; i++)
        queue_enqueue(q, i);

    for(i=0; i < test_len; i++)
        assert(queue_dequeue(q) == i);

    assert(queue_empty == true);
    queue_destroy(q);
    assert(q==nullptr);    
}


int test_bitmap()
{
    uint64_t len = (uint64_t) 65538;
    Bitmap bm = Bitmap_init(len, false);
    int i;
    bool err = false;


    for(i=0; i < len; i++)
    {
        if(GET_BIT(bm, i) != 0)
        {
            fprintf(TEST_OUT, "assertion 1 failed at i=%d\n", i);
            err = true;
            break;
        }

        SET_BIT(bm, i);

        if(GET_BIT(bm, i) == 0)
        {
            fprintf(TEST_OUT, "assertion 2 failed at i=%d\n", i);
            err = true;
            break;
        }

        CLR_BIT(bm, i);

        if((bool)GET_BIT(bm, i) != 0)
        {
            fprintf(TEST_OUT, "assertion 3 failed at i=%d\n", i);
            err = true;
            break;
        }
    }

    Bitmap_free(bm);
    fprintf(TEST_OUT, "finished\n");

    assert(err==false);
}