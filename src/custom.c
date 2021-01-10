#include "custom.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

queue_t queue_new(uint64_t max_len)
{
  //int a = sizeof(queue_t);
  //printf(a);
  queue_t q;
  q.head = 0;
  q.tail = 0;
  q.size = 0;
  q.data = (uint64_t*) malloc(max_len * sizeof(uint64_t));
  q.max_len = max_len;
  return q;
}

void queue_enqueue(queue_t* q, uint64_t data)
{
    assert(queue_size(q) <= q->max_len);
    q->data[q->tail] = data;
    q->tail++; 
    q->size = queue_size(q);
}

uint64_t queue_dequeue(queue_t* q)
{
    assert(!queue_empty(q));
    uint64_t retVal = q->data[q->head];
    q->head++;
    q->size = queue_size(q);
    return retVal;
}

bool queue_empty(queue_t* q)
{
  if (q->head == q->tail)
  {
    return true;
  }
  else
  {
    return false;
  }  
}

void queue_destroy(queue_t *q)
{
  free(q->data);
}

uint64_t queue_size(queue_t* q)
{
    return (q->tail - q->head);
}




Bitmap Bitmap_init(uint64_t bit_cnt, bool init_val)
{
	uint64_t mask, byte_cnt, uint64_cnt;	
	Bitmap bitmap = nullptr;

	byte_cnt = bit_cnt / BYTES_PER_LONG + BYTES_PER_LONG;
	uint64_cnt = byte_cnt / BYTES_PER_LONG;

	bitmap = (Bitmap)malloc(uint64_cnt); // TODO: multiply with sizeof
	assert(bitmap != nullptr);


	if(init_val == true) mask = UINT64_MAX;
	else mask = 0;
	memset(bitmap, mask, uint64_cnt*sizeof(bitmap[0]));	// TODO: parallize

	return bitmap;
}

void Bitmap_free(Bitmap bm)
{
	assert(bm != nullptr);
	free(bm);
	bm = nullptr;
}
