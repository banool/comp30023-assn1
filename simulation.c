#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

#define FCFS  "fcfs"
#define MULTI "multi"

/* Extern declarations: */
extern char *optarg;

void print_usage(char *program_name);
void simulate_fcfs(Process *disk_processes, Memory *memory);
void simulate_multi(Process *disk_processes, Memory *memory);
void step();

int main(int argc, char** argv)
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
				// set the value of size (int value) based on optarg
				target = optarg;
				break;

			case 'a':
				if(strcmp(optarg, FCFS) == 0) 
					alg = optarg;

				else if(strcmp(optarg, MULTI) == 0) 
					alg = optarg;

				else
				{
					// exit if optarg unknown
					fprintf(stderr, "Invalid scheduling option %s\n", optarg);
					print_usage(argv[0]);
					exit(1);
				}
 				break;
					// should not get here

			case 'm':
				// set the value of size (int value) based on optarg
				memsize = atoi(optarg);
				break;

			default:
				print_usage(argv[0]);
				return -1;
				break;
		}
	}	
	
	// a debugging step is included to display the value of args read
	//diag
	//printf("target = %s, algorithm = %s, size = %d\n", target, alg, memsize);
	
	// TODO insert error/validity checking for options.

	Process *disk_head = read_processes(target, memsize);
	if (disk_head == NULL) {
		fprintf(stderr, "%s could not be read in properly, exiting...\n", target);
		return -1;
	}

	//diag
	//print_processes_ll(disk_head);

	//printf("queue element 0 id = %d\n", fcfs->queue[2]->process_id);

	Memory *memory = create_memory(memsize);

	if (!strcmp(alg, FCFS))
		simulate_fcfs(disk_head, memory);
	else 
		simulate_multi(disk_head, memory);
	
	return 0;
}

void
print_usage(char *program_name) {
	fprintf(stderr, "Usage: %s -f [target file name] -a [algorithm] -m [memsize]\n", program_name);
	fprintf(stderr, "-f Name of input text file\n");
	fprintf(stderr, "-a [fcfs | multi]\n");
	fprintf(stderr, "-m Integer in mb greater than 0\n");
}


void
simulate_fcfs(Process *disk_processes, Memory *memory) {

	int timer = 0;
	int checked_future_processes;
	Process *check = disk_processes;

	Queue *fcfs = create_queue(-1);

	Process *active = NULL;

	while(1) {
		//diag
		//printf("Time %d num_items = %d\n", timer, fcfs->num_items);
		

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
				queue_insert(fcfs, check);
				if (check->next != NULL) {
					disk_processes = check->next;
				} else {
					disk_processes = NULL;
				}
			}
			// Testing if we are at the end of the list.
			if (check->next == NULL) {
				checked_future_processes = 1;
			} else {
				check = check->next;
			}
		}

		/*
		** Deciding which process to run. If there isn't an active process
		** we will pop the front of the queue and move it into memory/
		*/
		if (active == NULL) {
			active = queue_pop(fcfs);
			active->active = 1;

			// Checking if the process is in memory already. If not:
			if(!active->in_mem) {
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
					** linked list.
					*/
					Process *prev_head = disk_processes;
					disk_processes = memory_remove_largest(memory);
					prev_head->prev = disk_processes;
					disk_processes->next = prev_head;
					disk_processes->prev = NULL;
				}
			}

			memory_count_holes(memory);
			printf("time %d, %d running, numprocesses=%d, numholes=%d,\
 memusage=%d%%\n", timer, active->process_id, memory->num_processes,
memory->num_holes, (int)(((double)active->mem_size/memory->end)*100));
		}
		
		active->remaining_time -= 1;

		/*
		** If the active process hits finishes executing, we call the
		** function that finds it in memory, removes it and relinks the
		** neighbouring processes. We then free it and set active to NULL.
		*/
		if (active->remaining_time == 0) {
			// Discard the pointer, we don't need it as the process is done.
			memory_remove(memory, active->process_id);
			free_process(active);
			free(active);
			active = NULL;
			/* TODO insert a way to check if we're done. num_items in
			   queue being 0 might not be the best because another item
			   might come in while the queue is empty. check by future
			   processes being empty or something? */
			if (!fcfs->num_items) {
				// printf("death\n"); //diag
				timer += 1;
				break;
			}
		}

		timer += 1;
		check = disk_processes;
	}

	printf("time %d, simulation finished.\n", timer);
}

void simulate_multi(Process *disk_processes, Memory *memory) {
	
	int timer = 0;
	int checked_future_processes;
	Process *check = disk_processes;

	Queue *q1 = create_queue(Q1_LENGTH);
	Queue *q2 = create_queue(Q2_LENGTH);
	Queue *q3 = create_queue(Q3_LENGTH);
	
	Queue *curr_queue = q1;
	int remaining_quantum = q1->quantum;

	Process *active = NULL;

	while(1) {
		//diag
		//printf("Time %d q3 num_items = %d\n", timer, q3->num_items);
		
		//printf("curr q: %d", curr_queue->quantum);
		//diag
		//printf("    remaining quantum: %d\n", remaining_quantum);
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
		
		// fun fact. this line:
		// if (active != NULL && remaining_quantum > 0) {
		// enclosing this block makes it a fcfs alg instead.
		// Checking if the current queue has expired its quantum.
		if (remaining_quantum == 0) {
			// Move the active item onto the end of the next queue.
			//active->active = 0;
			Queue *next_queue = get_next_queue(curr_queue, q1, q2, q3);
			// If this is true, we've looped back to the shortest quantum.
			// We don't want our process to be looped back into the short
			// quantum, so we just leave it in the last queue.
			if (active != NULL) {
				if (next_queue->quantum == Q1_LENGTH) { 
					queue_insert(q3, active);
				} else {
					// In this case the active item must've just been in q1 or q2.
					// As such, we move it down into the next queue (q2 or q3).
					queue_insert(next_queue, active);
				}
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

			// Is this how round robin works? Just going back to q1 each time?
			// Doesn't seem as fair as how it functions without these 2 lines.
			//curr_queue = q1;
			//remaining_quantum = curr_queue->quantum;
			active = NULL;
		}
		
		//diag
		//printf("mem usage: %d\n", get_mem_usage(memory));
		/*
		** Deciding which process to run. If there isn't an active process
		** we will pop the front of the queue and move it into memory/
		*/
		if (active == NULL) {
			//diag
			//printf("popping new thign off the queueuueeuueuee\n");
			//diag
			//printf("quantum: %d  num items in queue: %d\n", curr_queue->quantum, curr_queue->num_items);
			active = queue_pop(curr_queue);
			// TODO when items pop from q3 the num items isnt decreasing.
			active->active = 1;
			
			// Checking if the process is in memory already. If not:
			if(!active->in_mem) {
				
				// Remove the process from disk as it is now in memory.
				Process *curr = disk_processes;
				if (curr->active) {
					//diag
						//printf("popping off head of disk\n");
					if (curr->next != NULL) {
						disk_processes = curr->next;
						disk_processes->prev = NULL;
					} else {
						//diag
						//printf("disk processes are null\n");
						disk_processes = NULL;
					}
				} else {
					//diag
						//printf("relink disk processes\n");
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
						//diag
						//printf("next\n");
						curr = curr->next;
					}
				}
				//diag
				//printf("hey\n");

				/* 
				** Trying to insert the process into memory.
				** If this fails, it calls the function to remove the
				** largest process currently memory.
				** It will keep doing these two functions until the 
				** process is inserted successfully.
				*/
				
				while (!memory_insert(memory, active)) {
					//diag
					//printf("current mem usage %d\n", get_mem_usage(memory));
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
						//diag
						//printf("current mem usage %d\n", get_mem_usage(memory));
						Process *curr = disk_processes;
						while (curr->next != NULL) {
							curr = curr->next;
						}
						curr->next = memory_remove_largest(memory);
						//printf("curr->id: %d\n", curr->process_id);
						curr->next->prev = curr;
						curr->next->next = NULL;
						//diag
						//printf("non null memers\n");
						/*
						Process *prev_head = disk_processes;
						disk_processes = 
						prev_head->prev = disk_processes;
						prev_head->next = NULL; //TODO this fucking line man lmao just a fresh ass infinite loop where the end would point back to the start what a killer.
						disk_processes->next = prev_head;
						disk_processes->prev = NULL;*/
					}
					//diag
					//printf("current mem usage %d\n", get_mem_usage(memory));
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
			//if (curr_queue->quantum == Q3_LENGTH)
			//	curr_queue->num_items -= 1;
			//diag
			//printf("active id: %d is dying\n", active->process_id);
			// Discard the pointer, we don't need it as the process is done.
			memory_remove(memory, active->process_id);
			free_process(active);
			free(active);
			active = NULL;
			remaining_quantum = 1;
			/* TODO insert a way to check if we're done. num_items in
			   queue being 0 might not be the best because another item
			   might come in while the queue is empty. check by future
			   processes being empty or something? */
			if (!q1->num_items && !q2->num_items && !q3->num_items && disk_processes == NULL) {
				// printf("death\n"); //diag
				timer += 1;
				break;
			}
			//diag
			//printf("it has died\n");
		}

		remaining_quantum -= 1;
		timer += 1;
		check = disk_processes;
	}

	printf("time %d, simulation finished.\n", timer);
}


void 
step_fcfs(Process *disk_processes, Memory *memory, Process *active) {
	// do shit
}


/*
** Thanks to user3707766 from https://stackoverflow.com/questions
** /2422712/c-rounding-integer-division-instead-of-truncating

int round_up_div(int a, int b) {
	int c = a/b;
	if (a % b) { c++; };
	return c;
}*/