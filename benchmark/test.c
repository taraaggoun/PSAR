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
static int node;
static off_t file_size;
static void *file_data = NULL;
static char *file_path = NULL;

/**
 * Set process on any core of node
*/
void set_core_node(void)
{
	struct bitmask *nodemask = numa_allocate_nodemask();
	numa_bitmask_setbit(nodemask, node);
	numa_run_on_node_mask(nodemask);
	numa_free_nodemask(nodemask);
}

/**
 * Set process on any core that is not in node
*/
void set_core_except_node(void)
{
	struct bitmask *nodemask = numa_allocate_nodemask();
	int nb_nodes = numa_num_configured_nodes();
	for (int i = 0; i < nb_nodes; i++)
		if (i != node)
			numa_bitmask_setbit(nodemask, i);
	numa_run_on_node_mask(nodemask);
	numa_free_nodemask(nodemask);
}

/**
 * Error managment
*/
void error(const char *msg)
{
	dprintf(STDERR_FILENO, "%s\n", msg);
	if (fd < 0)
		close(fd);
	if (file_data != NULL)
		munmap(file_data, file_size);
	exit(EXIT_FAILURE);
}

/**
 * Load a file in node page cache
*/
void load_memory(void)
{
	set_core_node();

	int fd = open(file_path, O_RDONLY);
	if (fd < 0)
		error("Error on open");

	struct stat file_info;
	if (stat(file_path, &file_info) != 0)
		error("Error on stat");

	file_size = file_info.st_size;
	void *file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (file_data == MAP_FAILED)
		error("Error on mmap");

	close(fd);
	fd = -1;
}

// /**
//  * Read the file randomly and calculate the reading time
// */
// void read_file(char *buf, size_t len, int type)
// {
// 	int fd = open(file_path, O_RDONLY);
// 	if (fd < 0)
// 		error("Error on open");

// 	// Get end of file
// 	int off_end = lseek(fd, 0, SEEK_END);

// 	struct timespec start = { 0 };
// 	struct timespec end = { 0 };

// 	// Get the start time
// 	if (clock_gettime(CLOCK_REALTIME, &start) < 0)
// 		error("Error on clock get start time");
	
// 	// Read randomly in the file
// 	for(int i = 0; i < 100000; i++) {
// 		// Choose a location randomly within the file
// 		int offset = rand() % off_end;
// 		lseek(fd, offset, SEEK_SET);
// 		read(fd, &buf, len);
// 	}

// 	// Get the end time
// 	if (clock_gettime(CLOCK_REALTIME, &end) < 0)
// 		error("Error on clock get end time");

// 	// Get current cpu
// 	cpu_set_t mask;
// 	if (sched_getaffinity(0, sizeof(mask), &mask) == -1)
// 		error("Error on get affinity");

// 	int cpu = -1;
// 	int nb_cpus = numa_num_configured_cpus();
// 	for (int i = 0; i < nb_cpus; i++) {
// 		if (CPU_ISSET(i, &mask)) {
// 			cpu = i;
// 			break;
// 		}
// 	}
// 	size_t time = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
// 	printf("Time of execution : %ld ns in core %d\n", time, cpu);
// 	close(fd);
// 	fd = open("benchmark.data", O_CREAT | O_APPEND | O_WRONLY, 0644);
// 	if (fd < 0)
// 		error("Error on open");
// 	char buff[12] = { 0 };
// 	snprintf(buff, 12,"%d %ld\n", type, time);
// 	if (write(fd, buff, 12) < 0)
// 		error("Error on write");
// 	close(fd);
// 	fd = -1;
// }

void read_file(char *buf, size_t len, ssize_t type)
{
	int fd = open(file_path, O_RDONLY);
	if (fd < 0)
		error("Error on open");

	struct timespec start = { 0 };
	struct timespec end = { 0 };

	// Get the start time
	if (clock_gettime(CLOCK_REALTIME, &start) < 0)
		error("Error on clock get start time");

	ssize_t total = 0;
	ssize_t r;
	while ((r = read(fd, buf + total, len - total)) > 0) {
		total += r;
	}
	if (total != len)
		error("Error: end of read we have not read the size of file");

	// Get the end time
	if (clock_gettime(CLOCK_REALTIME, &end) < 0)
		error("Error on clock get end time");
	
	// Get current cpu
	cpu_set_t mask;
	if (sched_getaffinity(0, sizeof(mask), &mask) == -1)
		error("Error on get affinity");
	int cpu = -1;
	int nb_cpus = numa_num_configured_cpus();
	for (int i = 0; i < nb_cpus; i++) {
		if (CPU_ISSET(i, &mask)) {
			cpu = i;
			break;
		}
	}
	size_t time = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
	printf("Time of execution : %ld ns in core %d\n", time, cpu);
	close(fd);
	
	fd = open("benchmark.data", O_CREAT | O_APPEND | O_WRONLY, 0644);
	if (fd < 0)
		error("Error on open");
	char buff[12] = { 0 };
	snprintf(buff, 12,"%d %ld\n", type, time);
	if (write(fd, buff, 12) < 0)
		error("Error on write");
	close(fd);
	fd = -1;
}

/**
 * Run the test on two different CPUs.
*/
void run_test(void)
{
	pid_t f = fork();
	if (f == -1)
		error("Error on fork");
	if (f == 0) {
		printf("------------ Test local-local ------------\n");
		char *buf = calloc(file_size, sizeof(char));
		read_file(buf, file_size, 0);
		munmap(file_data, file_size);
		exit(EXIT_SUCCESS);
	}
	wait(NULL);
	f = fork();
	if (f == -1)
		error("Error on fork");
	if (f == 0) {
	 	printf("----------- Test local-distant -----------\n");
		set_core_except_node();
		char *buf = calloc(file_size, sizeof(char));
		set_core_node();
		read_file(buf, file_size, 1);
	 	munmap(file_data, file_size);
	 	exit(EXIT_SUCCESS);
	}
	wait(NULL);
	f = fork();
	if (f == -1)
		error("Error on fork");
	if (f == 0) {
		printf("----------- Test distant-local -----------\n");
		set_core_except_node();
		char *buf = calloc(file_size, sizeof(char));
		read_file(buf, file_size, 2);
		munmap(file_data, file_size);
		exit(EXIT_SUCCESS);
	}
	wait(NULL);
	f = fork();
	if (f == -1)
		perror("Error on fork");
	if (f == 0) {
		printf("---------- Test distant-distant ----------\n");
		char *buf = calloc(file_size, sizeof(char));
		set_core_except_node();
		read_file(buf, file_size, 3);
		munmap(file_data, file_size);
		exit(EXIT_SUCCESS);
	}
	wait(NULL);
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

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("Usage: %s <file_path> <num node>\n", argv[0]);
		return EXIT_FAILURE;
	}

	fd = -1;
	node = atoi(argv[2]);
	file_path = argv[1];

	empty_caches();
	load_memory();
	run_test();
	
	munmap(file_data, file_size);
}
