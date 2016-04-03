#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "process.h"

#define BUFFSIZE 32

/*
** This function reads the input file and outputs an array
** of processes as if they were on disk. We can then treat
** this array is if it were current and future processes on disk.
** 
** Returns pointer to the head of the linked list of disk processes.
** 
** Initial code based off code from Andrew Turpin, writen
** Wed 29 Apr 2015 06:32:22 AEST
*/
Process *read_processes(char *target, int memsize)
{
    // This line confirms if the target file exists.
    if(access(target, R_OK ) == -1 ) {
        fprintf(stderr, "File could not be found.\n");
        return NULL;
    }

    // Saving the pointer to the first Process created.
    int first = 1;
    Process *head;

    // Keeping track of the most recently created process
    // so that we can point it to the next one we create.
    Process *recent;

    FILE *fp;
    fp = fopen(target, "r");

    char buff[BUFFSIZE];

    while ((fgets(buff, BUFFSIZE, fp) != NULL)) {
        // time_created, process_id, memory_size, job_time;
        int inp1, inp2, inp3, inp4;
        sscanf(buff, "%d %d %d %d", &inp1, &inp2, &inp3, &inp4);

        if (inp1 < 0 || inp2 <= 0 || inp3 <= 0 || inp4 <= 0) {
            fprintf(stderr, "Invalid input.\n");
            return NULL;
        }

        if (inp3 > memsize) {
            fprintf(stderr, "A process is too large for memory.\n");
            return NULL;
        }

        if (first) {
            head = create_process(inp1, inp2, inp3, inp4);
            head->prev = NULL;
            recent = head;
        } else {
            recent->next = create_process(inp1, inp2, inp3, inp4);
            recent->next->prev = recent;
            recent = recent->next;
        }
        
        first = 0;

    }

    fclose(fp);
    return head;
}

/*
** Creates a new process. Should only need to be called from whichever
** function reads in the initial input data, as these processes will
** then be loaded "on to disk", meaning a queue that acts as if the
** swap space on disk.
** 
** Parameters are: time_created, process_id, memory_size, job_time
*/
Process *create_process(int inp1, int inp2, int inp3, int inp4)
{
    Process *p = malloc(sizeof(Process));
    assert(p);

    // These are the inital given data.
    p->time_created = inp1;
    p->process_id = inp2;
    p->mem_size = inp3;
    p->job_time = inp4;

    // The following variables are only relevant when the process is in memory.
    // As such, many are set to placeholders like -1 or NULL.
    p->start = -1;
    p->end = -1;
    p->remaining_time = inp4; // Just the job_time to start with.
    p->active = 0;
    p->in_mem = 0;
    p->time_inserted_into_mem = -1;

    p->next = malloc(sizeof(Process*));
    p->next = NULL; // Will already be NULL if tail.
    p->prev = malloc(sizeof(Process*));
    p->prev = NULL; // Will already be NULL if head.

    return p;
}

/*
** Frees a process. This is called once a process finishes executing entirely.
*/
void free_process(Process *p)
{
    p->next = NULL;
    p->prev = NULL;
    free(p->next);
    free(p->prev);
}
