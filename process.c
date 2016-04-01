/*
 * This function reads the input file and outputs an array
 * of processes as if they were on disk. We can then treat
 * this array is if it were current and future processes on disk.
 * 
 * Returns pointer to the head of the linked list of disk processes.
 * 
 * Initial code based off code from Andrew Turpin, writen
 * Wed 29 Apr 2015 06:32:22 AEST
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "process.h"

#define BUFFSIZE 32

Process
*read_processes(char *target, int memsize) {

    FILE *fp;
    fp = fopen(target, "r");

    char buff[BUFFSIZE];

    // Saving the pointer to the first Process created.
    int first = 1;
    Process *head;

    /* Keeping track of the Process most recently created
       so that we can point it to the next one we create. */
    Process *recent;


    while ((fgets(buff, BUFFSIZE, fp) != NULL)) {
        // time_created, process_id, memory_size, job_time;
        int inp1, inp2, inp3, inp4;
        sscanf(buff, "%d %d %d %d", &inp1, &inp2, &inp3, &inp4);

        if (inp1 < 0 || inp2 <= 0 || inp3 <= 0 || inp4 <= 0) {
            fprintf(stderr, "Invalid input.\n");
            return NULL;
        }

        if (inp3 >= memsize) {
            fprintf(stderr, "A process is too large for memory.\n");
            return NULL;
        }

        if (first) {
            head = create_process(inp1, inp2, inp3, inp4);
            head->previous = NULL;
            recent = head;
        } else {
            recent->next = create_process(inp1, inp2, inp3, inp4);
            recent->next->previous = recent;
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
** Parameters are: time_created, process_id, memory_size, job_time
*/
Process
*create_process(int inp1, int inp2, int inp3, int inp4) {
    printf("hey\n");
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

    p->next = NULL; // Will stay NULL if tail.
    p->previous = NULL; // Will stay NULL if head.
    return p;
}

// TODO this might be redundant.
Process
*duplicate_process(Process *in) {
    Process *out = create_process(in->time_created, in->process_id, 
        in->mem_size, in->job_time);
    return out;
    
}

void
print_processes(Process *head) {

    if (head == NULL) {
        printf("Error: Head is NULL.\n");
        return;
    }

    Process *curr = head;

    int i = 0;
    while(1) {
        printf("Element %2d: time created = %2d, current->id = %2d, \
memory size = %3d, job time = %3d", i, curr->time_created, 
            curr->process_id, curr->mem_size, curr->job_time);
        if (curr-> previous != NULL)
            printf(", Previous id = %d\n", curr->previous->process_id);
        else {
            printf("\n");
        }
        if (curr->next == NULL) {
            break;
        }
        curr = curr->next;
        i += 1;
    }
}