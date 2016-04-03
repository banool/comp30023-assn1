#include <stdlib.h>
#include <assert.h>
#include "queue.h"

//TODO a smaller base queue size makes it not correctly track the start of
// the queue after it allocates more size for itself. Perhaps using
// incorrect logic to "math out" the start of the queue. 

Queue *create_queue(int quantum)
{
    Queue *q;
    /* 
    ** This odd malloc is due to how flexible/dynamic array members work
    ** inside a struct. You don't malloc the array separately like this:
    ** q->queue = malloc(sizeof(Process*) * BASE_QUEUE_SIZE);
    ** but instead just malloc the Queue the size of both the size of the
    ** struct AND the size of the array.
    */
    q = malloc(sizeof(Queue) + sizeof(Process*) * BASE_QUEUE_SIZE);
    q->quantum = quantum;
    q->start = 0;
    q->num_items = 0;
    q->max_size = BASE_QUEUE_SIZE;
    return q;
}

void queue_insert(Queue *q, Process *in)
{
    // Making sure the queue has enough space for the new element.
    // If not we double the queue size.
    // TODO this doesn't work. What if we made a new queue and copied
    // all items into the start of the new queue.
    if (q->max_size <= q->num_items) {
        q = realloc(q, sizeof(Queue) + sizeof(Process*) * q->max_size * 2);
        assert(q);
        q->max_size *= 2;
    }
    int next_index = (q->start + q->num_items) % q->max_size;
    q->queue[next_index] = in;
    q->num_items += 1;
}

Process *queue_pop(Queue *q)
{
    Process *ret = q->queue[q->start];
    q->queue[q->start] = NULL;
    int new_start = (q->start + 1) % q->max_size;
    q->start = new_start;
    
    q->num_items -= 1;
    return ret;
}


/*
** This function returns which queue is next for purposes of moving a process
** down in the queue order.
**
** Note that this solution isn't very extensible, and an array which holds
** a pointer to each queue would be much more efficient. However in a system 
** with a small, constant number of queues, this works fine.
*/
Queue *get_next_queue(Queue *curr_queue, Queue *q1, Queue *q2, Queue *q3)
{
    switch(curr_queue->quantum) {
        case Q1_LENGTH:
            return q2;
            break;
        case Q2_LENGTH:
            return q3;
            break;
        case Q3_LENGTH:
            // Just put it back on q3 as it is the last queue.
            return q3;
            break;
        // Should be impossible to get here.
        default:
            return NULL;
            break;
    }
}