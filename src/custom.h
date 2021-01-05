#ifndef CUSTOM_H
#define CUSTOM_H


#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define Bitmap uint64_t*
#define nullptr ((void *)0)
#define NULL nullptr

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

// QUEUE
// Taken from https://de.wikibooks.org/wiki/Algorithmen_und_Datenstrukturen_in_C/_Warteschlange
#define SUCCESS 0
#define ERR_INVAL 1
#define ERR_NOMEM 2

#define FALSE 0
#define TRUE 1

typedef struct queue_s queue_t;

int queue_destroy(queue_t *queue);
int queue_empty(queue_t *queue);
queue_t *queue_new(void);
void *queue_dequeue(queue_t *queue);
int queue_enqueue(queue_t *queue, void *data);

struct queue_node_s {
  struct queue_node_s *next;
  void *data;
};

struct queue_s {
  struct queue_node_s *front;
  struct queue_node_s *back;
};


 int queue_destroy(queue_t *queue) {
   if (queue == NULL) {
     return ERR_INVAL;
   }
   while (queue->front != NULL) {
     struct queue_node_s *node = queue->front;
     queue->front = node->next;
     free(node);
   }
  free(queue);
  return SUCCESS;
}

int queue_empty(queue_t *queue) {
  if (queue == NULL || queue->front == NULL) {
    return TRUE;
  } else {
    return FALSE;
  }
}

queue_t *queue_new(void) {
  queue_t *queue = malloc(sizeof(*queue));
  if (queue == NULL) {
    return NULL;
  }
  queue->front = queue->back = NULL;
  return queue;
}

void *queue_dequeue(queue_t *queue) {
	if (queue == NULL || queue->front == NULL) {
	return NULL;
	}
	struct queue_node_s *node = queue->front;
	void *data = node->data;
	queue->front = node->next;
	if (queue->front == NULL) {
	queue->back = NULL;
	}
	free(node);
	return data;
}

int queue_enqueue(queue_t *queue, void *data) {
  if (queue == NULL) {
    return ERR_INVAL;
  }
  struct queue_node_s *node = malloc(sizeof(*node));
  if (node == NULL) {
    return ERR_NOMEM;
  }
  node->data = data;
  node->next = NULL;
  if (queue->back == NULL) {
    queue->front = queue->back = node;
  } else {
    queue->back->next = node;
    queue->back = node;
  }
  return SUCCESS;
}



#endif