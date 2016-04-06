#include "process.h"

#define Q1_LENGTH 2
#define Q2_LENGTH 4
#define Q3_LENGTH 8
#define BASE_QUEUE_SIZE 64

typedef struct Queue_s {
    int quantum; // -1 means fcfs.
    /*
    ** The array is used cyclically. When an item is popped from the front,
    ** start is progressed one, so the first item on the queue is moved to
    ** the next item on the array. If another item is added, it is put at the
    ** end of the queue relative to where it starts. E.g.
    ** If start is at index 1, the max_size is 4 and there are 3 elements, the 
    ** new element will be put at index 0. New element index calculated with:
    ** >> (start index + number of elements) % array length
    ** ie (start + num_elements) % max_size
    */
    int start;
    int num_items;
    int max_size;
    // An array of pointers to processes (either in memory or on "disk").
    Process *queue[];
} Queue;

Queue *create_queue(int quantum);
void queue_insert(Queue *q, Process *in);
Process *queue_pop(Queue *q);
Queue *get_next_queue(Queue *curr_queue, Queue *q1, Queue *q2, Queue *q3);