#include <stdlib.h>
#include <assert.h>
#include "memory.h"

//diag we dont need this, just here for printf
#include <stdio.h>
// TODO Justify this choice. It is 3 current for testing purposes.
#define BASE_QUEUE_SIZE 30
//TODO a smaller base queue size makes it not correctly track the start of
// the queue after it allocates more size for itself. Perhaps using
// incorrect logic to "math out" the start of the queue. 

/*
** Creates the "memory" as it were. Must be made with a process
** given to it first. This isn't a huge concern because we don't
** need Memory until a process is spawned, and a process is always
** spawned at time 0. TODO REVISE THIS. YOU NOW DONT NEED A PROCESS
*/
Memory
*create_memory(int mem_size) {
    Memory *mem = malloc(sizeof(Memory));
    assert(mem);
    mem->start = 0;
    mem->end = mem_size;
    mem->processes = NULL;
    mem->num_processes = 0;
    mem->num_holes = 0;
    return mem;
}

// Returns 1 on success, 0 on failure. If failure we use memory_insert_full.
int
memory_insert(Memory *mem, Process *in) {
    //printf("inserting to memorry\n");
    
    // If there are no items in memory yet.
    if (mem->processes == NULL) {
        mem->processes = in;

        in->start = 0;
        in->end = in->mem_size;
        in->next = NULL; //TODO process.c:64:66 explains why we do this.
        in->prev = NULL; //TODO is this necessary?

        mem->num_processes += 1;
        in->in_mem = 1;
        return 1;

    }

    Process *curr = mem->processes;
    // Checking for space between start of memory and first process.
    if ((mem->start + in->mem_size) < curr->start) {
        in->start = 0;
        in->end = in->mem_size;

        in->prev = NULL;
        in->next = mem->processes;
        mem->processes = in;

        mem->num_processes += 1;
        in->in_mem = 1;
        return 1;
    }

    // Checking for space between each process.
    while (curr->next != NULL) {
        // Checking for a large enough gap.
        if ((curr->end + in->mem_size) < curr->next->start - 2) {
            curr->next->prev = in;
            in->next = curr->next;
            curr->next = in;
            in->prev = curr;

            in->start = curr->end + 1;
            in->end = in->start + in->mem_size;

            mem->num_processes += 1;
            in->in_mem = 1;
            return 1;
        }
        curr = curr->next;
    }

    //diag
    //printf("curr end: %d\n", curr->end);
    // Checking for space between last process and end.
    if ((curr->end + in->mem_size) < (mem->end - 2)) {
        curr->next = in;
        in->prev = curr;
        in->next = NULL;

        in->start = curr->end + 1;
        in->end = in->start + in->mem_size;

        mem->num_processes += 1;
        in->in_mem = 1;
        return 1;
    }

    // If we get here, it means there was no space and we have to
    // swap something to disk. Call memory_insert_full().
    return 0;
}

// TODO EXPAND this pre much is used when it's been found that there
// isn't enough space in memory. This function finds the id of the largest
// item in memory and then passes it to memory_remove() to remove it.
Process
*memory_remove_largest(Memory *mem) {
    
    //diag
    //printf("removing largest\n");

    Process *curr = mem->processes;

    // Checking for space between each process.
    int id_biggest = curr->process_id;
    while (curr->next != NULL) {
        // Checking for a large enough gap.
        if (curr->next->mem_size > curr->mem_size) {
            id_biggest = curr->next->process_id;
        }
        //printf("curr target: %d\n", id_biggest);
        curr = curr->next;
    }
    //diag
    //printf("removing largest iddasdsad: %d\n", id_biggest);
    return memory_remove(mem, id_biggest);
}


/* 
** Returns the Process in question. We do this because we remove a process
** from memory in two cases:
** 1. The process has completed. If this is the case we can just free
**    the process and discard it.
** 2. The process has been pulled from memory to make space for something
**    else but it hasn't completed. As such we need to pointer so we 
**    can re-add it to disk.
*/
Process
*memory_remove(Memory *mem, int process_id) {
    Process *curr = mem->processes;

    // Checking if there was only one item in memory.
    // If so, we free/remove it and reset mem->processes to NULL.
    //printf("eat my memes awtch them grow num items %d\n", mem->num_processes);
    if (mem->num_processes == 1) {
        Process *ret = mem->processes;
        ret->active = 0;
        ret->in_mem = 0;
        mem->processes = NULL;
        mem->num_processes -= 1;
        return ret;
    }

    // Checking if the process to remove is after the 1st element.
    int first = 1;
    while(1) {
        //diag
        //printf("entered here tho\n");
        //printf("target %d and curr %d\n", process_id, curr->process_id);
        if (curr->process_id == process_id) {
            
            curr->active = 0;
            curr->in_mem = 0;
            // Linking the neighbouring processes.
            if (curr->prev != NULL) {
                curr->prev->next = curr->next;
            }
            if (curr->next != NULL) {
                curr->next->prev = curr->prev;
            }
            mem->num_processes -= 1;
            //if (first)
            //    mem->processes = curr->next;
            return curr;
        }

        curr = curr->next;
        first = 0;

        // Checking for the end of the linked list.        
        //if(curr->next == NULL) {
        //    break;
        //}
    }
    
    // Shouldn't get here.
    printf("you got here memed\n");
    return NULL;
}

void 
memory_count_holes(Memory *mem) {
    int holes = 0;
    
    Process *curr = mem->processes;
    if (curr == NULL) {
        mem->num_holes = 1;
    }

    // Checking if the first process is at mem address 0 or not.
    if (curr->start > mem->start)
    {
        holes +=1;
    }
    
    while(1) {
        // printf("start: %d, end %d\n", curr->start, curr->end); //diag
        // Checking for the end of the list.
        if(curr->next == NULL) 
            break;
        // Checking for holes up until the last process.
        if (curr->end < curr->next->start - 1) 
        {
            holes +=1;
        }

        curr = curr->next;
    }
    
    // Checking for a hole between the last process and the end.
    if (curr->end < mem->end)
    {
        //diag once done with make this a non-bracketed single statement.
        //printf("curr end: %d and mem end %d\n", curr->end, mem->end);
        holes +=1;
    }

    mem->num_holes = holes;
}

int
get_mem_usage(Memory *mem) {
    int total_usage = 0;

    Process *curr = mem->processes;
    if (curr == NULL) {
        return 0;
    }

    while(1) {
        total_usage += curr->mem_size;
        if (curr->next == NULL) {
            break;
        }
        curr = curr->next;
    }
    
    return ((int)(((double)total_usage/mem->end)*100));
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
    //diag
    //printf("Index for insert of process %d: %d\n", in->process_id, next_index);
    q->queue[next_index] = in;
    q->num_items += 1;
    
}

Process
*queue_pop(Queue *q) {
    Process *ret = q->queue[q->start];
    q->queue[q->start] = NULL;
    //diag
    //printf("%d pre current number of items: %d\n", q->quantum, q->num_items);
    q->num_items -= 1;
    //printf("%d post current number of items: %d\n", q->quantum, q->num_items);
    int new_start = (q->start + 1) % q->max_size;
    //diag
    //printf("New start index: %d\n", new_start);
    q->start = new_start;
    return ret;
}


// TODO note how this is n't very extensible, and an array which holds
// a pointer to each queue would be much mroe efficient.
Queue
*get_next_queue(Queue *curr_queue, Queue *q1, Queue *q2, Queue *q3) {
    switch(curr_queue->quantum) {
        case Q1_LENGTH:
            return q2;
            break;
        case Q2_LENGTH:
            return q3;
            break;
        case Q3_LENGTH:
            return q1;
            break;
        // Should be impossible to get here.
        default:
            return NULL;
            break;
    }
}