#include <stdlib.h>
#include <assert.h>
#include "memory.h"

/*
** Creates the "memory" as it were. Holes aren't formally tracked but
** are easily implied from the space between each process (or between
** a process and the start/end of memory).
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

// Returns 1 on success, 0 on failure. If failure we use memory_remove_largest.
int memory_insert(Memory *mem, Process *in)
{
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
        if ((curr->end + in->mem_size) < curr->next->start) {
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

    // Checking for space between last process and end.
    if ((curr->end + in->mem_size) <= (mem->end)) {
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
Process *memory_remove_largest(Memory *mem)
{
    Process *curr = mem->processes;

    // Checking for space between each process.
    int id_biggest = curr->process_id;
    int biggest = curr->mem_size;
    while (curr->next != NULL) {
        // Checking for a large enough gap.
        if (curr->next->mem_size > biggest) {
            id_biggest = curr->next->process_id;
            biggest = curr->next->mem_size;
        }
        curr = curr->next;
    }
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
Process *memory_remove(Memory *mem, int process_id)
{
    Process *curr = mem->processes;

    // Checking if there was only one item in memory.
    // If so, we free/remove it and reset mem->processes to NULL.
    //printf("eat my memes awtch them grow num items %d\n", mem->num_processes);
    if (mem->processes->next == NULL) {
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
            if (first)
                mem->processes = curr->next;
            return curr;
        }
        curr = curr->next;
        first = 0;
    }
    // Shouldn't get here.
    return NULL;
}

void memory_count_holes(Memory *mem)
{
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
    if (curr->end < mem->end) {
        holes +=1;
    }

    mem->num_holes = holes;
}

int get_mem_usage(Memory *mem)
{
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
