#include "custom.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define TEST_OUT stderr

int main()
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

    if(!err)
    {
        fprintf(TEST_OUT, "All tests passed\n");
        return 0;
    }
    else
    {
        return -1;
    }
}
