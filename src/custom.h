#ifndef CUSTOM_H
#define CUSTOM_H


#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define Bitmap uint64_t*
#define nullptr ((void *)0)

#define BITS_PER_LONG 64
#define BYTES_PER_LONG 8

// Taken from https://stackoverflow.com/questions/2525310/how-to-define-and-work-with-an-array-of-bits-in-c
// TODO: check if integer dicision can be sped up
#define SET_BIT(bitmap,k) ( bitmap[(k)/BITS_PER_LONG] |=  (1UL << ((k) % BITS_PER_LONG)) )
#define CLR_BIT(bitmap,k) ( bitmap[(k)/BITS_PER_LONG] &= ~(1UL << ((k) % BITS_PER_LONG)) )
#define GET_BIT(bitmap,k) ( bitmap[(k)/BITS_PER_LONG] &   (1UL << ((k) % BITS_PER_LONG)) )


Bitmap Bitmap_init(uint64_t bit_cnt, bool init_val)
{
	uint64_t mask, byte_cnt, uint64_cnt;	
	Bitmap bitmap = nullptr;

	byte_cnt = bit_cnt / BYTES_PER_LONG + BYTES_PER_LONG;
	uint64_cnt = byte_cnt / BYTES_PER_LONG;

	bitmap = (Bitmap)malloc(uint64_cnt);
	assert(bitmap != nullptr);


	if(init_val == true) mask = UINT64_MAX;
	else mask = 0;
	memset(bitmap, mask, uint64_cnt*sizeof(bitmap[0]));

	return bitmap;
}

void Bitmap_free(Bitmap bm)
{
	assert(bm != nullptr);
	free(bm);
	bm = nullptr;
}

#endif