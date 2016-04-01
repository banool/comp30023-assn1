#include <stdlib.h>
#include <assert.h>
#include "memory.h"

//diag we dont need this, just here for printf
#include <stdio.h>
// TODO Justify this choice. It is 3 current for testing purposes.
#define BASE_QUEUE_SIZE 3

/*
** Creates the "memory" as it were. Must be made with a process
** given to it first. This isn't a huge concern because we don't
** need Memory until a process is spawned, and a process is always
** spawned at time 0. TODO REVISE THIS. YOU NOW DONT NEED A PROCESS
*/
Memory
*create_memory(int memsize) {
    Memory *mem = malloc(sizeof(Memory));
    assert(mem);
    mem->start = 0;
    mem->end = memsize;
    mem->processes = NULL;
    mem->num_processes = 0;
    return mem;
}

Queue
*create_queue(int quantum) {
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

void
queue_insert(Queue *q, Process *in) {
    // Making sure the queue has enough space for the new element.
    if (q->max_size <= q->num_items) {
        //diag
        printf("Increasing queue max size.\n");
        q = realloc(q, sizeof(Queue) + sizeof(Process*) * q->max_size * 2);
        assert(q);
        q->max_size *= 2;
    }
    int next_index = (q->start + q->num_items) % q->max_size;
    printf("Index for insert of process %d: %d\n", in->process_id, next_index);
    q->queue[next_index] = in;
    q->num_items += 1;
    
}

Process
*queue_pop(Queue *q) {
    Process *ret = q->queue[q->start];
    q->queue[q->start] = NULL;
    q->num_items -= 1;
    int new_start = (q->start + 1) % q->max_size;
    q->start = new_start;
    return ret;
}