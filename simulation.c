#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "memory.h"
#include "queue.h"

#define FCFS  "fcfs"
#define MULTI "multi"

/* Extern declarations: */
extern char *optarg;

int num_swaps = 0;

void print_usage(char *program_name);
void simulate(Process *disk_processes, int num_procceses, Memory *memory, 
	char *alg);

int main(int argc, char **argv)
{
	char input;
	char *target;	// Target input file name.
	char *alg;		// fcfs or multi.
	int  memsize;	// Given virtual memsize.

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
		}
	}
	
	/*
	** Here we read processes into disk. Throughout the life of the
	** simulation, this disk linked list will hold both processes that
	** haven't yet been started (future processes whose start times are
	** greater than the current simulation time) as well as processes
	** that have been swapped back to disk because memory was too full.
	*/
	int num_processes = 0;
	Process *disk_head = read_processes(target, memsize, &num_processes);
	if (disk_head == NULL) {
		fprintf(stderr, "%s couldn't be read properly, exiting...\n", target);
		return -1;
	}

	Memory *memory = create_memory(memsize);

	simulate(disk_head, num_processes, memory, alg);
	
	return 0;
}


/*
** Print usage on incorrect command line usage.
*/
void print_usage(char *program_name)
{
	fprintf(stderr, "Usage: %s -f [target file name] -a [algorithm]\
 -m [memsize]\n", program_name);
	fprintf(stderr, "-f Name of input text file\n");
	fprintf(stderr, "-a [fcfs | multi]\n");
	fprintf(stderr, "-m Integer in mb greater than 0\n");
}

/*
** This is the main function that controls the simulation in the program.
** After we create the appropriate queues, the main steps of this function are:
** - Check for processes on disk which haven't been loaded into a queue yet
**     based on if the timer equals the start time for that process.
** - If alg is multi, check if the current queue has expired its quantum.
**     If so, we move the active process (assuming there is one and it hasn't
**     just finished running) back onto the appropriate queue and mark the next
**     queue to run.
** - If there isn't an active process, we pop one off the queue and remove
**     it from disk (if not already in memory).
** - We try to load this process in to memory. If there isn't enough space,
**     we keep removing the largest (oldest if equal sizes) from the queue 
**     until there is enough space.
** - Count the holes and print the statement for when a process starts running.
** - Decrement the time remaining for the active process and then check if it
**     is done. If so, we remove the process from memory and free it.
** - If a process just finished, we then check if there are any processes
**     left in the queues or on disk. If so, we're done!
** - Otherwise, we just incrememnt the timer and decrement the remaining
**     time for the given quantum (assuming we're doing multi).
*/
void simulate(Process *disk_processes, int num_processes, Memory *memory,
	char *alg)
{
	int timer = 0;
	int checked_future_processes;
	Process *check = disk_processes;

	Queue *q1;

	// Setting up for the given algorithm.
	int fcfs = 0;
	if (!strcmp(alg, FCFS)) {
		fcfs = 1;
		q1 = create_queue(-1, num_processes); // -1 Signifies a fcfs queue.
	} else {	
		q1 = create_queue(Q1_LENGTH, num_processes);
	}
	
	// These will be unsued if the algorithm is fcfs.
	Queue *q2 = create_queue(Q2_LENGTH, num_processes);
	Queue *q3 = create_queue(Q3_LENGTH, num_processes);
	
	Queue *curr_queue = q1;
	int remaining_quantum = q1->quantum;

	Process *active = NULL;

	while(1) {

		check = disk_processes;
		
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
			}
			
			// Testing if we are at the end of the list.
			if (check->next == NULL) {
				checked_future_processes = 1;
			} else {
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
				active = NULL;
			}
			
			// Round robin into the next queue for this iteration.
			// q1, then q2 if q1 empty, then q3 if q2 empty.
			curr_queue = q1;
			if (!q1->num_items) {
				curr_queue = q2;
			}
			if (!q1->num_items && !q2->num_items) {
				curr_queue = q3;
			}

			remaining_quantum = curr_queue->quantum;
		}
		
		/*
		** Deciding which process to run. If there isn't an active process
		** we will pop the front of the queue and move it off disk into memory.
		*/
		if (active == NULL) {

			/*
			** If we get here, it means all the queues are empty. However,
			** because the block at the end of the script hasn't exited the
			** loop yet, it means there must still be processes yet to start.
			** As such, we just increment timer and move to the next interval.
			*/
			if (curr_queue->num_items == 0) {
				timer += 1;
				remaining_quantum = 0;
				continue;
			}

			// Pop the item from the current queue and mark it as active.
			active = queue_pop(curr_queue);
			active->active = 1;

			// Checking if the process is in memory already. If not we remove
			// it from disk and later insert it into memory.
			if(!active->in_mem) {
				Process *curr = disk_processes;
				// Checking if the relevant process is at the head of the
				// disk linked list.
				if (curr->active) {
					// If there is more than one item, link the head to the
					// next process in the linked list.
					if (curr->next != NULL) {
						disk_processes = curr->next;
						disk_processes->prev = NULL;
					// Otherwise just set disk processes to NULL. 
					} else {
						disk_processes = NULL;
					}
				// If the process is not at the head, we iterate through
				// disk processes until we find it and re-link accordingly.
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
				while (!memory_insert(memory, active, timer)) {
					/*
					** Remove the largest and put it back on disk.
					** Doesn't matter where we put it back on disk because
					** the arrival part won't try to queue up processes that
					** should have already arrived (arrival time < timer).
					** As such we just add it to the tail of the disk processes
					** linked list. 
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

			// Count holes and print the appropriate message for when
			// a process starts running.
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
			// If so, we're done!
			
			if (!q1->num_items && !q2->num_items && !q3->num_items && 
				disk_processes == NULL) {
				timer += 1;
				break;
			}
		}
		remaining_quantum -= 1;
		timer += 1;
	}

	printf("time %d, simulation finished.\n", timer);
	//printf("num_swaps %d\n", num_swaps);
}
