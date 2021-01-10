#ifndef CUSTOM_H
#define CUSTOM_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define Bitmap uint64_t*
#define nullptr ((void *)0)
//#define NULL nullptr

#define BITS_PER_LONG 64
#define BYTES_PER_LONG 8

// Taken from https://stackoverflow.com/questions/2525310/how-to-define-and-work-with-an-array-of-bits-in-c
// TODO: check if integer dicision can be sped up
#define SET_BIT(bitmap,k) ( bitmap[(k)/BITS_PER_LONG] |=  (1UL << ((k) % BITS_PER_LONG)) )
#define CLR_BIT(bitmap,k) ( bitmap[(k)/BITS_PER_LONG] &= ~(1UL << ((k) % BITS_PER_LONG)) )
#define GET_BIT(bitmap,k) ( bitmap[(k)/BITS_PER_LONG] &   (1UL << ((k) % BITS_PER_LONG)) )

typedef struct queue_s queue_t;



struct queue_s {
  uint64_t head;
  uint64_t tail;
  uint64_t size;
  uint64_t max_len;
  uint64_t* data;
};

Bitmap Bitmap_init(uint64_t bit_cnt, bool init_val);
void Bitmap_free(Bitmap bm);

queue_t queue_new(uint64_t max_size);
void queue_enqueue(queue_t* q, uint64_t data);
uint64_t queue_dequeue(queue_t* q);
bool queue_empty(queue_t* q);
void queue_destroy(queue_t *q);
uint64_t queue_size(queue_t* q);

#endif