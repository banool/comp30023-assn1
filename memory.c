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

/*
** First checks for space between the end of memory and the first process.
** Then checks for space between each process. Finally looks for space between
** the last process and the start of memory. Links pointers appropriately.
**
** Returns 1 on success, 0 on failure. If failure we use memory_remove_largest.
*/
int memory_insert(Memory *mem, Process *in, int timer)
{
    // If there are no items in memory yet.
    if (mem->processes == NULL) {
        mem->processes = in;

        in->start = mem->end - in->mem_size;
        in->end = mem->end;

        in->next = NULL; //TODO process.c:64:66 explains why we do this.
        in->prev = NULL; //TODO is this necessary?

        mem->num_processes += 1;
        in->in_mem = 1;
        in->time_inserted_into_mem = timer;
        return 1;
    }

    Process *curr = mem->processes;

    // Checking for space between first process and end.
    if ((mem->end - in->mem_size) >= (curr->end)) {
        in->prev = NULL;
        in->next = mem->processes;
        mem->processes = in;
        curr->prev = in;

        in->start = mem->end - in->mem_size;
        in->end = mem->end;

        mem->num_processes += 1;
        in->in_mem = 1;
        in->time_inserted_into_mem = timer;
        return 1;
    }

    // Checking for space between each process.
    while (curr->next != NULL) {
        // Checking for a large enough gap.
        if ((curr->start - in->mem_size) >= curr->next->end) {
            in->start = curr->start - in->mem_size;
            in->end = curr->start;

            curr->next->prev = in;
            in->next = curr->next;
            curr->next = in;
            in->prev = curr;

            mem->num_processes += 1;
            in->in_mem = 1;
            in->time_inserted_into_mem = timer;
            return 1;
        }
        curr = curr->next;
    }

    // Checking for space between start of memory and last process.
    if ((curr->start - in->mem_size) >= mem->start) {
        in->start = curr->start - in->mem_size;
        in->end = curr->start;

        curr->next = in;
        in->prev = curr;
        in->next = NULL;

        mem->num_processes += 1;
        in->in_mem = 1;
        in->time_inserted_into_mem = timer;
        return 1;
    }

    // If we get here, it means there was no space and we have to
    // swap something to disk. Call memory_insert_full().
    return 0;
}

/*
** Iterates through all processes in memory. Sets the biggest as the 
** first process and then updates it if each next process is bigger.
** In the case of an equal size, the process that has been in their
** longer is marked for deletion. Once the largest process is found, 
** it is passed to memory_remove() for deletion.
*/
Process *memory_remove_largest(Memory *mem)
{
    Process *curr = mem->processes;

    // Checking for space between each process.
    int id_biggest = curr->process_id;
    int biggest = curr->mem_size;
    int time_inserted = curr->time_inserted_into_mem;

    while (curr->next != NULL) {
        // Checking for a large enough gap.
        if (curr->next->mem_size > biggest) {
            id_biggest = curr->next->process_id;
            biggest = curr->next->mem_size;
            time_inserted = curr->time_inserted_into_mem;
        } else 
        // This means that there is another process that is equally large.
        // As per the spec we select whichever has been in memory longest.
        if (curr->next->mem_size == biggest) {
            if (curr->next->time_inserted_into_mem < time_inserted) {
                id_biggest = curr->next->process_id;
            }
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
    if (mem->processes->next == NULL) {
        Process *ret = mem->processes;
        ret->active = 0;
        ret->in_mem = 0;
        ret->time_inserted_into_mem = -1;
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
            curr->time_inserted_into_mem = -1;
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

/*
** Recalculates and updates num_holes value in the memory struct.
** Iterate through the processes and check the space between each of them. 
*/
void memory_count_holes(Memory *mem)
{
    int holes = 0;
    
    Process *curr = mem->processes;
    if (curr == NULL) {
        mem->num_holes = 1;
    }

    // Checking if the first process is at mem address <end> or not.
    if (curr->end < mem->end) {
        holes +=1;
    }
    
    while(1) {
        // Checking for the end of the list.
        if(curr->next == NULL) 
            break;
        // Checking for holes up until the last process.
        if (curr->start > curr->next->end) 
        {
            holes +=1;
        }
        curr = curr->next;
    }
    
    // Checking for a hole between the last process and the end.
    if (curr->start > mem->start) {
        holes +=1;
    }

    mem->num_holes = holes;
}

/*
** Add up the total size of the processes in emory and return
** as a percentage. For use the print function when a process starts.
*/
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
