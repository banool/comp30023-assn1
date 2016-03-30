
#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>


/* Extern declarations: */

extern  int     optind;
extern  char    *optarg;

int main(int argc, char** argv)
{
	char input;
	char *language = "C";	// default language name
	int size = 100;			// default size (or number of lines)

	/* parse command line options example
	 
	   an appropraite usage message should be included
	 
	   a sample command line is provided below:
	
					./prog_name -l Java -s 500
	 
	   where l option is used to specify the name of the language used (char array)
	         s option is used to specify the size or the number of lines in the file (int)
	 
	*/
	
	
	while ((input = getopt(argc, argv, "l:s:")) != EOF)
	{
		switch ( input )
		{
			case 'l':
				if(strcmp(optarg, "Java") == 0) 
					language  = optarg;

				else if(strcmp(optarg, "Python") == 0) 
					language  = optarg;

				else if(strcmp(optarg, "C") != 0) 
				{
					// exit if optarg unknown
					fprintf(stderr, "Unknown %s\n ", optarg);
					exit(1);
				}
 				break;
					// should not get here
           
			case 's':
				// set the value of size (int value) based on optarg
				size = atoi(optarg);
				break;

			default:
				// do something
				break;
		} 
	}
	
	// ideally, a usage statement should also be included
	
	// a debugging step is included to display the value of args read
	printf("language = %s size = %d\n", language, size);
		
	// once the input arguments have been processed, call other functions
	// after appropriate error checking
	
	return 0;
}
