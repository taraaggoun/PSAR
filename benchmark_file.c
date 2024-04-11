#include <time.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

/**
 * Take in argument a file name and optinally one or two core number
 * load the file in memory and try to read it from two separatly core
*/

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
 * Read the file filepath sequentially to load it into memory
*/
int load_memory(char *filepath) 
{
	int fd = open(filepath, O_RDONLY);
	if (fd == -1) {
		printf("The argument need to be a valid path to the file \n");
		return -1;
	}
	char buf;

	off_t end = lseek(fd, 0, SEEK_END);
	// Sequential reading of the file to map it into memory
	lseek(fd, 0, SEEK_SET);
	while (1) {
		int ret = read(fd, &buf, 1);
		if (ret == 0) {
			break;
		}
		if (ret == -1) {
			printf("Error while reading the file \n");
			return -1;
		}
		// Move the pointer to the next page
		// Return NULL if we reach the end of the file
		off_t cur =  lseek(fd, 1 << 12, SEEK_CUR);
		if (cur == end || cur == 0)
			break;
	}
	return fd;
}

/**
 * Read the file randomly and calculate the reading time
*/
void read_file(int fd, int cpu) 
{
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
	printf("Time of execution : %d in core %d\n", (int)(time_end - time_start), cpu);
}

/**
 * Run the test on two different CPUs.
*/
int run_test(int cpu1, int cpu2, int fd)
{
	pid_t f = fork();
	if (f == -1) {
		printf("Error on fork\n");
		return -1;
	}
	if (f == 0) { // Child
		// Change core of child
		if (set_core(cpu1) < -1) {
			printf("Error set_core run_test\n");
			return -1;
		}
		read_file(fd, cpu1);
		exit(EXIT_SUCCESS);
	}
	else { // Parent
		read_file(fd, cpu2);
		wait(NULL);
	}
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	int cpu1 = 3;
	int cpu2 = 6;
 
	// argument manager
	if (argc < 2 || argc > 4) {
		printf("Error, it need at least one argument : <FilePath> !\n");
		printf("Optionally one or two integer for the <name of core>\n");
		return 1;
	}
	if (argc >= 3)
		cpu1 = atoi(argv[2]);
	if (argc == 4)
		cpu2 = atoi(argv[3]);

	// Assign process to a core
	if (set_core(cpu1) < 0) {
		printf("Error set_core main\n");
		return 1;
	}

	// load file in memory
	int fd = load_memory(argv[1]);
	if ( fd == -1)
		return 1;

	printf("---------- Test local-local ----------\n");
	if (run_test(cpu1, cpu2, fd) < 0) {
		close(fd);
		return 1;
	}

	printf("---------- Test local-distant ----------\n");
	if (run_test(cpu1, cpu2, fd) < 0) {
		close(fd);
		return 1;
	}

	close(fd);
	return 0;
}