#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

/* Extern declarations: */
extern char *optarg;

void print_usage(char *program_name);

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
				if(strcmp(optarg, "fcfs") == 0) 
					alg = optarg;

				else if(strcmp(optarg, "multi") == 0) 
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
	printf("target = %s, algorithm = %s, size = %d\n", target, alg, memsize);
	
	// TODO insert error/validity checking for options.

	Process *disk_head = read_processes(target, memsize);
	if (disk_head == NULL) {
		fprintf(stderr, "%s could not be read in properly, exiting...\n", target);
		return -1;
	}

	print_processes(disk_head);

	Memory *memory = create_Memory(memsize, disk_head);
	
	return 0;
}

void
print_usage(char *program_name) {
	fprintf(stderr, "Usage: %s -f [target file name] -a [algorithm] -m [memsize]\n", program_name);
	fprintf(stderr, "-f Name of input text file\n");
	fprintf(stderr, "-a [fcfs | multi]\n");
	fprintf(stderr, "-m Integer in mb greater than 0\n");
}
