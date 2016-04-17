// Daniel Porteous porteousd

#include <stdlib.h>
#include <assert.h>
#include "queue.h"

/*
** Creates a new queue with the given quantum.
** Note the comment about mallocing size for a struct with a
** flexible (dynamic) array member.
** 
** Sets the queue size to the maximum number of items it could hold
** being the number of processes that were given initially.
*/
Queue *create_queue(int quantum, int size)
{
    Queue *q;
    /* 
    ** This odd malloc is due to how flexible/dynamic array members work
    ** inside a struct. You don't malloc the array separately like this:
    ** q->queue = malloc(sizeof(Process*) * BASE_QUEUE_SIZE);
    ** but instead just malloc the Queue the size of both the size of the
    ** struct AND the size of the array.
    */
    q = malloc(sizeof(Queue) + sizeof(Process*) * size);
    q->quantum = quantum;
    q->start = 0;
    q->num_items = 0;
    q->max_size = size;
    return q;
}

/*
** Calculates where to put the next item. This calculation takes into
** account the cyclical nature of the queue array using modulo.
*/
void queue_insert(Queue *q, Process *in)
{
    int next_index = (q->start + q->num_items) % q->max_size;
    q->queue[next_index] = in;
    q->num_items += 1;
}

/*
** Pops an item from the front of the queue and adjusts the start
** accordingly. This is either incrementing by 1 or looping back
** to the start of the array if we're at the end.
*/
Process *queue_pop(Queue *q)
{
    Process *ret = q->queue[q->start];
    q->queue[q->start] = NULL;
    q->start = (q->start + 1) % q->max_size;;
    
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
            // Just put it back on q3 as this is the last queue.
            return q3;
            break;
        // Should be impossible to get here.
        default:
            return NULL;
            break;
    }
}