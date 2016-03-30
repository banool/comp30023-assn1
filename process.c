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

Disk_process
*read_processes(char *target) {

    FILE *fp;
    fp = fopen(target, "r");

    char buff[BUFFSIZE];

    // Saving the pointer to the first Disk_process created.
    int first = 1;
    Disk_process *head;

    /* Keeping track of the Disk_process most recently created
       so that we can point it to the next one we create. */
    Disk_process *recent;


    while ((fgets(buff, BUFFSIZE, fp) != NULL)) {
        // time_created, process_id, memory_size, job_time;
        int inp1, inp2, inp3, inp4;
        sscanf(buff, "%d %d %d %d", &inp1, &inp2, &inp3, &inp4);

        if (inp1 < 0 || inp2 <= 0 || inp3 <= 0 || inp4 <= 0) {
            fprintf(stderr, "Invalid input.\n");
            return NULL;
        }

        if (first) {
            head = create_Disk_process(inp1, inp2, inp3, inp4);
            recent = head;
        } else {
            recent->next = create_Disk_process(inp1, inp2, inp3, inp4);
            recent = recent->next;
        }
        
        first = 0;

    }

    fclose(fp);
    return head;
}

Disk_process
*create_Disk_process(int inp1, int inp2, int inp3, int inp4) {
    printf("hey\n");
    Disk_process *dp = malloc(sizeof(Disk_process));
    assert(dp);
    dp->time_created = inp1;
    dp->process_id = inp2;
    dp->memory_size = inp3;
    dp->job_time = inp4;
    dp->next = NULL;
    return dp;
}

void
print_process_ids(Disk_process *head) {

    if (head == NULL) {
        printf("Error: Head is NULL.\n");
        return;
    }

    Disk_process *current = head;

    int i = 0;
    while(1) {
        printf("Element %d: id = %d\n", i, current->process_id);
        if (current->next == NULL) {
            break;
        }
        current = current->next;
        i += 1;
    }
}