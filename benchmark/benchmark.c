#include <numa.h>
#include <time.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <numaif.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

static int fd;
static int node1;
static int node2;
static int node3;
static off_t file_size;
static void *file_data = NULL;
static char *file_path = NULL;

/**
 * Error managment
*/
void error(const char *msg)
{
	dprintf(STDERR_FILENO, "Error : %s\n", msg);
	if (fd >= 0)
		close(fd);
	if (file_data != NULL)
		munmap(file_data, file_size);
	exit(EXIT_FAILURE);
}

/**
 * Empty caches
*/
void empty_caches() {
	int fd = open("/proc/sys/vm/drop_caches", O_WRONLY);
	if (write(fd, "1", 1) < 0)
		error("Error on write");
	close(fd);
	fd = -1;
}

/**
 * Set process on any core of node
*/
void set_core_node(int node)
{
	struct bitmask *nodemask = numa_allocate_nodemask();
	numa_bitmask_setbit(nodemask, node);
	numa_run_on_node_mask(nodemask);
	numa_free_nodemask(nodemask);
}


/**
 * Read the file filepath sequentially to load it into memory
*/
void load_memory(void) 
{
	fd = open(file_path, O_RDONLY);
	if (fd == -1) {
		printf("The argument need to be a valid path to the file \n");
		return;
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
			return;
		}
		// Move the pointer to the next page
		// Return NULL if we reach the end of the file
		off_t cur =  lseek(fd, 1 << 12, SEEK_CUR);
		if (cur == end || cur == 0)
			break;
	}
	close(fd);
}

int main(int argc, char *argv[]) {
	if (argc != 5) {
		printf("Usage: %s <file_path> <num node P> <num node B> <num node R>\n", argv[0]);
		return EXIT_FAILURE;
	}

	fd = -1;
	file_path = argv[1];
	node1 = atoi(argv[2]);
	node2 = atoi(argv[3]);
	node3 = atoi(argv[4]);

	struct stat file_info;
	if (stat(file_path, &file_info) != 0)
		error("Error on stat");
	file_size = file_info.st_size;

	empty_caches();
	set_core_node(node1);
	load_memory();
	int f = fork();
	if (f == -1)
		error("Error on fork");
	if (f == 0) {
		set_core_node(node2);
		char *buffer = numa_alloc_onnode(file_size, node2);
		memset(buffer, 0xff, file_size);
		set_core_node(node3);
	
		struct timespec start = { 0 };
		struct timespec end = { 0 };

		fd = open(file_path, O_RDONLY);
		if (fd == -1)
			error("Error on open");
		// Get the start time
		if (clock_gettime(CLOCK_REALTIME, &start) < 0)
			error("Error on clock get start time");
		for (int i = 0; i < 1000; i++) {
			int nb_read = 0;
			while(1) {
				int r = read(fd, buffer + nb_read, 1 << 13);
				if (r == -1)
					error("Error on read");
				if (r < 1 << 13)
					break;
				nb_read += r;
			}

		}
		// Get the end time
		if (clock_gettime(CLOCK_REALTIME, &end) < 0)
			error("Error on clock get end time");
		size_t time = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
		printf("Time of execution : %ld ns %d\n", time);

	}
	else {
		wait(NULL);
	}

}