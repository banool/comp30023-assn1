/*typedef struct Process_s  {
    int start; // The start address inside Memory.
    int size; // The given size of the process.
    int end; // Derived from start + size. Must be less than end of Memory.
    struct Process_s *previous;
    struct Process_s *next;
    
    ** Here we have multiple options. For the actual information of the process, do I just create
    ** a pointer to the Disk_process and access the information that way? Or do I clone in the
    ** data from the Disk_process here as their own indiviudal attributes of the struct. The
    ** former would result in greater efficiency, less space wastage and less potential difference
    ** in input data, but the later would be simpler and prevent multidepth pointer access like:
    ** memory->process[i]->disk_process.process_id
    
} Process; */

#include "process.h"

typedef struct Memory_s {
    int start; // Most likely 0.
    int end; // Will be the given memsize;
    int num_processes;
    // Just a pointer to the first process in the linked list.
    Process *processes;
} Memory;

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
    int num_elements;
    int max_size;
    // An array of pointers to processes (either in memory or on "disk").
    Process *queue[]; 
} Queue;

Memory *create_Memory(int memsize, Process *first_process);


