#include "process.h"

typedef struct Memory_s {
    int start; // Most likely 0.
    int end; // Will be the given memsize;
    int num_processes;
    int num_holes;
    // Just a pointer to the first process in the linked list.
    Process *processes;
} Memory;

Memory *create_memory(int mem_size);
int memory_insert(Memory *mem, Process *in, int timer);
Process *memory_remove_largest(Memory *mem);
Process *memory_remove(Memory *mem, int process_id);
void memory_count_holes(Memory *mem);
int get_mem_usage(Memory *mem);