#include <stdlib.h>
#include <assert.h>
#include "queue.h"

void queue_double(Queue *in);

/*
** Creates the queue struct. Important to note that allocating memory for
** a dynamic array in a struct (flexible array members) is different.
** This is further explained inside the struct.
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
** Inserts a new item in to the queue. Allocates additional space if needed.
** Because of the cycical nature of the queue it also recalculates the new
** start of the queue, which may wrap around to the start.
*/
void queue_insert(Queue *q, Process *in)
{   
    /*
    ** Making sure the queue has enough space for the new element.
    ** If not, we double the queue size.
    ** 
    ** Note: The new index calculation didn't work properly, so a
    ** high base queue size of 64 is allocated. If the queue needs 
    ** to be made larger, the program will eventually segfault.
    ** 
    ** This was the only known fault in the program, so a generous
    ** base queue size of 64 is just set initially.
    */
    printf("quantum: %d max size %d num items %d\n", q->quantum, q->max_size, q->num_items);
    if (q->max_size == q->num_items) {
        printf("increasing size\n");
        queue_double(q);
        /*printf("increasing size\n");
        //Queue *old = q;
        Queue *new = create_queue(q->quantum, q->max_size * 2);
        assert(new);
        new->num_items = q->num_items;
        int old_ind;
        for (int i = 0; i < q->max_size; i++) {
            old_ind = (q->start + i) % q->max_size;
            new->queue[i] = q->queue[old_ind];
            printf("old ind %d to new ind %d\n", old_ind, i);
        }
        q = NULL;
        free(q);
        q = new;
        q->max_size = q->max_size * 2;*/
    }
    int next_index = (q->start + q->num_items) % q->max_size;
    q->queue[next_index] = in;
    q->num_items += 1;
    printf("quantum: %d max size %d num items %d and next ind %d\n", q->quantum, q->max_size, q->num_items, next_index);
}

void queue_double(Queue *in)
{
    Process **old_q = malloc(sizeof(Process*) * in->max_size);
    assert(old_q);

    int old_ind;
    for (int i = 0; i < in->max_size; i++) {
        old_ind = (in->start + i) % in->max_size;
        old_q[i] = in->queue[old_ind];
    }

    //int old_quantum = in->quantum;
    int old_num_items = in->num_items;
    in = create_queue(in->quantum, in->max_size * 2);
    assert(in);
    //in->quantum = old_quantum;
    in->num_items = old_num_items;
    //in->max_size *= 2;

    for (int i = 0; i < in->max_size/2; i++) {
        in->queue[i] = old_q[i];
    }
}

/*
** Pops an item off the queue and recalculates where to put the next item.
*/
Process *queue_pop(Queue *q)
{   
    printf("quantum: %d pop baby from ind %d max size %d\n", q->quantum, q->start, q->max_size);
    printf("num items: %d\n", q->num_items);
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
        case Q1_QUANTUM:
            return q2;
            break;
        case Q2_QUANTUM:
            return q3;
            break;
        case Q3_QUANTUM:
            // Just put it back on q3 as it is the last queue.
            return q3;
            break;
        // Should be impossible to get here.
        default:
            return NULL;
            break;
    }
}