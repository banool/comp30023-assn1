#pragma once

typedef struct Process_s {
    // These are the inital given data.
    int time_created;
    int process_id;
    int mem_size;
    int job_time;
    
    // These variables are only relevant when the process is in memory.
    int start; // The start address inside Memory.
    int end; // Derived from start + mem_size. Must be less than end of Memory.
    int remaining_time; // Starts equal to job time.
    int active; // Bool representing if the Process is being actively run.
    int in_mem; // Bool representing if the Process is loaded into memory.
    int time_inserted_into_mem; // Value of timer when put into mem. -1 if out.

    /*
    ** We need to track both the next and previous process because
    ** when we delete a process, we need to link the previous and next
    ** process to each other.
    */
    struct Process_s *next;
    struct Process_s *prev;
} Process;

Process * read_processes(char *target, int memsize, int *num_processes);
Process *create_process(int inp1, int inp2, int inp3, int inp4);
void free_process(Process *p);