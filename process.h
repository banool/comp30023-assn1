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

    /*
    ** We need to track both the next and previous process because
    ** when we delete a process, we need to link the previous and next
    ** process to each other.
    */
    struct Process_s *next;
    struct Process_s *previous;
} Process;

Process *read_processes(char *target, int memsize);
Process *create_process(int inp1, int inp2, int inp3, int inp4);
void print_processes(Process *head);