#include <numa.h>
#include <time.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

static int fd = -1;
static int node = 0;
static int file_size = 0;
static char *file_data = NULL;
static char *file_path = NULL;

/**
 * Error managment
*/
static void error(const char *msg)
{
	dprintf(STDERR_FILENO, "%s\n", msg);
	if (fd < 0)
		close(fd);
	if (file_data != NULL)
		munmap(file_data, file_size);
	exit(EXIT_FAILURE);
}

/**
 * Set process on any core of node
*/
static void set_core_node()
{
	struct bitmask *nodemask = numa_allocate_nodemask();
	if (nodemask == NULL)
		error("mask null in set core node");
	numa_bitmask_setbit(nodemask, node);
	numa_run_on_node_mask(nodemask);
	numa_free_nodemask(nodemask);
}

/**
 * Set process on any core that is not in node
*/
static void set_core_except_node()
{
	struct bitmask *nodemask = numa_allocate_nodemask();
	if (nodemask == NULL)
		error("nodemask null in set core except node");
	int nb_nodes = numa_num_configured_nodes();
	for (int i = 0; i < nb_nodes; i++)
		if (i != node)
			numa_bitmask_setbit(nodemask, i);
	numa_run_on_node_mask(nodemask);
	numa_free_nodemask(nodemask);
}

static void set_core(int is_local) {
	if (is_local)
		set_core_node(node);
	else
		set_core_except_node(node);
}

/**
 * Empty page caches
*/
static void empty_caches(void) {
	fd = open("/proc/sys/vm/drop_caches", O_WRONLY);
	if (fd == -1)
		error("open drop_caches");
	if (write(fd, "1", 1) < 0)
		error("write in empty_cache");
	close(fd);
	fd = -1;
}

void load_memory_read(void) {
	fd = open(file_path, O_RDONLY);
	if (fd == -1)
		error("open in load memory with a read");

	// Get EOF
	off_t end = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	char buf;
	// Sequential reading of the file to map it into memory
	for (int i = 0; i < 10; i++) {
		while (1) {
			int r = read(fd, &buf, 1);
			if (r == 0)
				break;
			if (r == -1)
				error("read in load memory with a read");

			// Move the pointer to the next page
			off_t cur =  lseek(fd, 1 << 12, SEEK_CUR);
			if (cur < 0)
				error("lseek to go to next page");

			// Return NULL if we reach the end of the file
			if (cur == end)
				break;
		}
	}
	close(fd);
	fd = -1;
}

void load_memory_mmap(void) {
	fd = open(file_path, O_RDONLY);
	if (fd < 0)
		error("open in load memory with mmap");

	file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (file_data == MAP_FAILED)
		error("Error on mmap");

	close(fd);
	fd = -1;
}

void read_file(char *buf)
{
	fd = open(file_path, O_RDONLY);
	if (fd < 0)
		error("open in read sequentially");

	int read_size = 0;
	int size_left = file_size;
	struct timespec start = { 0 };
	struct timespec end = { 0 };

	// Get the start time
	if (clock_gettime(CLOCK_MONOTONIC, &start) < 0)
		error("Error on clock get start time");
	while (1) {
		int read_len = (size_left > 0x2000) ? 0x2000 : size_left;
		int r = read(fd, buf + r, read_len);
		if (r < 0)
			error("read file");
		if (r == 0)
			break;
		size_left -= r;
		read_size += r;
	}
	// Get the end time
	if (clock_gettime(CLOCK_MONOTONIC, &end) < 0)
		error("Error on clock get end time");
	size_t time = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
	printf("Time of read séquentielle: %ld ns ", time);

	close(fd);
	fd = -1;
}

void read_file_alea(char *buf)
{
	fd = open(file_path, O_RDONLY);
	if (fd < 0)
		error("open read file alea");
	
	// Retrieve the start time
	struct timespec start = { 0 };
	if (clock_gettime(CLOCK_MONOTONIC, &start) < 0)
		error("clock get start time\n");

	// Read randomly in the file
	for(int i = 0; i < 10000; i++) {
		// Choose a location randomly within the file
		int offset = rand() % file_size;
		// int read_len = rand() % (file_size - off_end);
		lseek(fd, offset, SEEK_SET);
		read(fd, buf, 0x2000);
	}

	// Get the end time
	struct timespec end = { 0 };
	if (clock_gettime(CLOCK_MONOTONIC, &end) < 0)
		error("clock get end time\n");
	printf("Time of read alea : %ld ns ", (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec));

	close(fd);
	fd = -1;
}

void print_node(void) {
	struct bitmask *mask = numa_allocate_cpumask();
	if (mask == NULL)
		error("mask null in print node");
	if (numa_sched_getaffinity(getpid(), mask) == -1) {
		numa_free_cpumask(mask);
		error("getaffinity in print node\n");
	}

	int cpu;
	for (cpu = 0; cpu < numa_num_configured_cpus(); cpu++)
		if (numa_bitmask_isbitset(mask, cpu))
			break;

	numa_free_cpumask(mask);
	
	if (numa_node_of_cpu(cpu) == node)
		printf("in local\n");
	else
		printf("in distant\n");
}

void get_read_time(int node_r, int mode)
{
	empty_caches();
	set_core(1);
	load_memory_read();
	
	pid_t f = fork();
	if (f < 0)
		error("fork");
	if (f == 0) { // Child
		printf("pid : %d\n", getpid);
		sleep(5);
		char *buffer = numa_alloc_onnode(file_size, node_r);
		memset(buffer, 0, file_size);

		set_core(node_r);
		if (mode)
			read_file(buffer);
		else
			read_file_alea(buffer);
		print_node();
		numa_free(buffer, file_size);
		exit(EXIT_SUCCESS);
	}
	wait(NULL);
}

int main(int argc, char *argv[])
{
	// Parameters managment
	if (argc != 3) {
		printf("Usage: %s <file_path> <num node>\n", argv[0]);
		return EXIT_FAILURE;
	}

	fd = -1;
	file_path = argv[1];
	node = atoi(argv[2]);

	srand(time(NULL));

	// Get file_size
	struct stat st;
	if (stat(file_path, &st) != 0)
		error("Error on stat");
	file_size = st.st_size;

	printf("------------ Test local ------------\n");
	get_read_time(1, 1);
	get_read_time(1, 0);
	printf("---------- Test distant ----------\n");
	get_read_time(0, 1);
	get_read_time(0, 0);

	return 0;
}