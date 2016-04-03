#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "queue.h"

#define FCFS  "fcfs"
#define MULTI "multi"

/* Extern declarations: */
extern char *optarg;

void print_usage(char *program_name);
void simulate(Process *disk_processes, Memory *memory, char *alg);

int main(int argc, char **argv)
{
	char input;
	char *target;	// Target input file name.
	char *alg;		// fcfs or multi.
	int  memsize;	// Given virtual memsize.

	// TODO is this how it should be done? Or with getopt also?
	if (argc != 7) {
		print_usage(argv[0]);
		return -1;
	}
	
	// Getting the options for program execution.
	while ((input = getopt(argc, argv, "f:a:m:")) != EOF)
	{
		switch ( input )
		{

			case 'f':
				// Ascertaining which file to read the input from.
				target = optarg;
				break;

			case 'a':
				// Ascertaining which algorithm to use.
				if(strcmp(optarg, FCFS) == 0) 
					alg = optarg;

				else if(strcmp(optarg, MULTI) == 0) 
					alg = optarg;

				else {
					// Exit if optarg unknown
					fprintf(stderr, "Invalid scheduling option %s\n", optarg);
					print_usage(argv[0]);
					exit(1);
				}
 				break;

			case 'm':
				// Ascertaining total memory size.
				memsize = atoi(optarg);
				break;

			default:
				// Should not get here.
				print_usage(argv[0]);
				return -1;
				break;
		}
	}	
	
	// TODO insert error/validity checking for options.

	Process *disk_head = read_processes(target, memsize);
	if (disk_head == NULL) {
		fprintf(stderr, "%s could not be read in properly, exiting...\n", target);
		return -1;
	}

	Memory *memory = create_memory(memsize);

	simulate(disk_head, memory, alg);
	
	return 0;
}

void print_usage(char *program_name)
{
	fprintf(stderr, "Usage: %s -f [target file name] -a [algorithm] -m [memsize]\n", program_name);
	fprintf(stderr, "-f Name of input text file\n");
	fprintf(stderr, "-a [fcfs | multi]\n");
	fprintf(stderr, "-m Integer in mb greater than 0\n");
}


void simulate(Process *disk_processes, Memory *memory, char *alg)
{
	int timer = 0;
	int checked_future_processes;
	Process *check = disk_processes;

	Queue *q1;

	int fcfs = 0;
	if (!strcmp(alg, FCFS)) {
		fcfs = 1;
		q1 = create_queue(-1); // -1 Signifies a fcfs queue.
	} else {	
		q1 = create_queue(Q1_LENGTH);
	}
	
	// These will be unsued if the algorithm is fcfs.
	Queue *q2 = create_queue(Q2_LENGTH);
	Queue *q3 = create_queue(Q3_LENGTH);
	
	Queue *curr_queue = q1;
	int remaining_quantum = q1->quantum;

	Process *active = NULL;

	while(1) {

		// Checking for any potential processes to be added to the queue.
		checked_future_processes = 0;
		if (check == NULL) 
			checked_future_processes = 1;
		while (!checked_future_processes) {
			/*
			** Is == and not >= because this would pick up processes that
			** have already been queued up once and then moved back to disk.
			** If these were triggered by this, then they would be duplicated
			** in the queue which is not what we want at all.
			*/
			if (check->time_created == timer) {
				queue_insert(q1, check);
				/*if (check->next != NULL) {
					disk_processes = check->next;
					disk_processes->prev = NULL;
				} else {
					disk_processes = NULL;
				}*/
			}
			// Testing if we are at the end of the list.
			if (check->next == NULL) {
				checked_future_processes = 1;
			} else {
				//printf("check next id: %d\n", check->next->process_id);
				check = check->next;
			}
		}
		
		/*
		** Checking if the current queue has expired its quantum.
		** We only bother with this block dealing with quantums and
		** multiple queues if the algorithm is multi. 
		** fcfs doesn't have to worry about this.
		*/
		if (!fcfs && remaining_quantum == 0) {
			// Figure out which queue is next after the current one.
			Queue *next_queue = get_next_queue(curr_queue, q1, q2, q3);

			// Active is sometimes NULL already because a process might
			// have finished before expiring its quantum.
			if (active != NULL) {
				// Returns q3 if it is already in q3.
				// We don't want our process to be looped back into the short
				// quantum, so we just leave it in the last queue.
				queue_insert(next_queue, active);
			}
			
			// Round robin into the next queue for this iteration.
			curr_queue = q1;
			if (!q1->num_items) {
				curr_queue = q2;
			}
			if (!q1->num_items && !q2->num_items) {
				curr_queue = q3;
			}

			remaining_quantum = curr_queue->quantum;
			active = NULL;
		}
		
		/*
		** Deciding which process to run. If there isn't an active process
		** we will pop the front of the queue and move it off disk into memory.
		*/
		if (active == NULL) {

			// Pop the item from the current queue and mark it as active.
			active = queue_pop(curr_queue);
			active->active = 1;

			// Checking if the process is in memory already. If not:
			if(!active->in_mem) {
				// Remove the process from disk as it is now in memory.
				Process *curr = disk_processes;
				if (curr->active) {
					if (curr->next != NULL) {
						disk_processes = curr->next;
						disk_processes->prev = NULL;
					} else {
						disk_processes = NULL;
					}
				} else {
					while (curr->next != NULL) {
						if (curr->next->active) {
							if (curr->next->next != NULL) {
								curr->next = curr->next->next;
								curr->next->prev = curr;
							} else {
								curr->next = NULL;
							}
							break;
						}
						curr = curr->next;
					}
				}

				/* 
				** Trying to insert the process into memory.
				** If this fails, it calls the function to remove the
				** largest process currently memory.
				** It will keep doing these two functions until the 
				** process is inserted successfully.
				*/				
				while (!memory_insert(memory, active)) {
					/*
					** Remove the largest and put it back on disk.
					** Doesn't matter where we put it back on disk because
					** the arrival part won't try to queue up processes that
					** should have already arrived (arrival time < timer).
					** As such we just add it to the head of the disk processes
					** linked list. EDIT TODO we now add it to the tail.
					*/
					if (disk_processes == NULL) {
						disk_processes = memory_remove_largest(memory);
						disk_processes->prev = NULL;
						disk_processes->next = NULL;
					} else {
						Process *curr = disk_processes;
						while (curr->next != NULL) {
							curr = curr->next;
						}
						curr->next = memory_remove_largest(memory);
						curr->next->prev = curr;
						curr->next->next = NULL;
					}
				}
			}
			
			memory_count_holes(memory);
			printf("time %d, %d running, numprocesses=%d, numholes=%d,\
 memusage=%d%%\n", timer, active->process_id, memory->num_processes,
memory->num_holes, get_mem_usage(memory));
		}
		
		active->remaining_time -= 1;

		/*
		** If the active process finishes executing, we call the
		** function that finds it in memory, removes it and relinks the
		** neighbouring processes. We then free it and set active to NULL.
		*/
		if (active->remaining_time == 0) {
			// Discard the pointer, we don't need it as the process is done.
			memory_remove(memory, active->process_id);
			free_process(active);
			free(active);
			active = NULL;
			remaining_quantum = 1;

			// Check if there's anything left in the queue or on disk.
			if (!q1->num_items && !q2->num_items && !q3->num_items && 
				disk_processes == NULL) {
				timer += 1;
				break;
			}
		}
		remaining_quantum -= 1;
		timer += 1;
		check = disk_processes;
	}
	printf("time %d, simulation finished.\n", timer);
}
