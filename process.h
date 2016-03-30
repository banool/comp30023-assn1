typedef struct Disk_process_s {
    int time_created;
    int process_id;
    int memory_size;
    int job_time;
    struct Disk_process_s *next;
} Disk_process;

Disk_process *read_processes(char *target);
Disk_process *create_Disk_process(int inp1, int inp2, int inp3, int inp4);
void print_process_ids(Disk_process *head);