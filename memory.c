#include <stdlib.h>
#include <assert.h>
#include "memory.h"


#define BASE_NUM_PROCESSES 4

/*
** Creates the "memory" as it were. Must be made with a process
** given to it first. This isn't a huge concern because we don't
** need Memory until a process is spawned, and a process is always
** spawned at time 0.
*/
Memory
*create_Memory(int memsize, Process *first_process) {
    Memory *mem = malloc(sizeof(Memory));
    assert(mem);
    mem->start = 0;
    mem->end = memsize;
    mem->processes = malloc(sizeof(Process));
    assert(mem->processes);
    mem->processes = NULL;
    mem->num_processes = 0;
    return mem;
}

