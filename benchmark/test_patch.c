#include <time.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

/**
 * Assign the program to the core: cpu
*/
int set_core(int cpu)
{
	cpu_set_t set;
	CPU_ZERO(&set);
	// Add to set the number of proc
	CPU_SET(cpu, &set);

	// Assign the program to all cores contained in the set
	if (sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
		printf("Error on setaffinity\n");
		return -1;
	}
	return 0;
}

/**
 * Read the file randomly and calculate the reading time
*/
void read_file(char *filename) 
{
	int fd = open(filename, O_RDONLY);
	if (fd == -1) {
		printf("The argument need to be a valid path to the file \n");
		return;
	}

	// Retrieve the start time
	clock_t time_start = clock();

	char buf;
	// Get end of file
	int end = lseek(fd, 0, SEEK_END);

	// Read randomly in the file
	for(int i = 0; i < 10000; i++) {
		// Choose a location randomly within the file
		int offset = rand() % end;
		lseek(fd, offset, SEEK_SET);
		read(fd, &buf, 1);
	}

	// Get the end time
	clock_t time_end = clock();
	printf("Time of execution : %d in core %d\n", (int)(time_end - time_start));
	close(fd);
}

int main(int argc, char *argv[])
{
	if (argc == 3) {
		printf("Error, bad number of argument \n Please give a filename and a number of core");
		return 1;
	}

	int core = atoi(argv[2]);
	set_core(core);
	read_file(argv[1]);

	return 0;
}