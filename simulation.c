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
	Queue *q1 = create_queue(2);
	Queue *q2 = create_queue(4);
	Queue *q3 = create_queue(8);
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