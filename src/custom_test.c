#include "custom.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define TEST_OUT stderr

int test_bitmap();
int test_queue();



int main()
{
   //test_bitmap();
   test_queue();
   fprintf(TEST_OUT, "All tests passed\n");
}

int test_queue()
{
    uint64_t test_len = 100;
    uint64_t i; 
    queue_t q = queue_new(test_len);

    assert(queue_empty(&q));
    assert(queue_size(&q) == 0);
    
    
    uint64_t test_vals[test_len];
    for(i=0; i < test_len; i++)
        test_vals[i] = i;

    for(i=0; i < test_len; i++)
    {
        queue_enqueue(&q, i);
        assert(!queue_empty(&q));
        assert(queue_size(&q) == i+1);
        assert(q.size == i+1);
    }
        

    for(i=0; i < test_len; i++)
    {
        assert(!queue_empty(&q));
        uint64_t val = queue_dequeue(&q);
        assert(val == i);
    }
    assert(queue_empty(&q));
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
    

    assert(err==false);
}