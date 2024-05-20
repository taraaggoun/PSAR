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
#include <sys/ioctl.h>
#include <sys/types.h>

// IOCTL
#define IOCTL_MAGIC 'N'
#define PIDW _IOW(IOCTL_MAGIC, 0, char*)

// GLOBAL VARIABLE
static int fd = -1;
static int fd_data = -1;
static int config = 1;
static int node = 0;
static int file_size = 0;
static char *file_data = NULL;
static char *file_path = NULL;

/**
 * Error managment
*/
static void error(const char *msg)
{
	dprintf(STDERR_FILENO, "Error: %s\n", msg);
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

/**
 * Set process in adequat core
*/
static void set_core(int is_local) {
	if (is_local)
		set_core_node();
	else
		set_core_except_node();
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
	printf("cache are empty\n");
}

/**
 * If c equal 0 disable numa balencing
 * If c equal 1 enable numa balencing
*/
static void write_numa_balencing(char c) {
	fd = open("/proc/sys/kernel/numa_balancing", O_WRONLY);
	if (fd == -1)
		error("open numa balencing");
	if (write(fd, &c, 1) < 0)
		error("write in numa balencing");
	close(fd);
	fd = -1;
	printf("%c write in /proc/sys/kernel/numa_balancing\n", c);
}

/**
 * Insert the pid in the kernel array with ioctl
*/
void insert_pid_ioctl(pid_t pid) {
	int fd = open("/dev/openctl", O_WRONLY);
	if (fd == -1)
		error("open openctl");

	if (ioctl(fd, PIDW, pid) == -1)
		error("ioctl");

	close(fd);
	fd = -1;
	printf("%d add in liste\n", pid);
}

/**
 * Load the file in memory
*/
void load_memory(void) {
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
	printf("file load in memory\n");
}

/**
 * Load all the n files in memory
*/
void load_memory_multiple(int n) {
	int fds[n];
	for (int i = 0; i < n; i++) {
		char pathname[13] = { 0 };
		snprintf(pathname, 13, "res/file%d", i + 1);
		fds[i] = open(pathname, O_RDONLY);
		if (fds[i] < 0)
			error("open fds");
	}
	printf("All file are open\n");
	// Get EOF
	off_t end = lseek(fds[0], 0, SEEK_END);
	lseek(fds[0], 0, SEEK_SET);

	char buf;
	// Sequential reading all the file to map them into memory
	int is_done = 0;
	while (1) {
		for (int i = 0; i < n; i++) {
			int r = read(fds[i], &buf, 1);
			if (r == 0)
				is_done = 1;
			if (r == -1)
				error("read in load memory with a read");
			// Move the pointer to the next page
			off_t cur =  lseek(fds[i], 1 << 12, SEEK_CUR);
			if (cur < 0)
				error("lseek to go to next page");
			// Return NULL if we reach the end of the file
			if (cur == end)
				is_done = 1;
		}
		if (is_done)
			break;
	}
	for (int i = 0; i < n; i++)
		close(fds[i]);
	printf("all file are load in memory\n");
}

/**
 * Read a file sequencially and get the time in a file
*/
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
	printf("Time of read séquentielle: %ld ns \n", time);
	char buffer[200];
	snprintf(buffer, 200, "%lu\n", time);
	write(fd_data,buffer,strlen(buffer));
	close(fd);
	fd = -1;
}

/**
 * Read a file randomnly
 * get the time in a file
*/
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
	size_t time = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
	printf("Time of read alea : %ld ns \n", time);
	char buffer[200];
	snprintf(buffer, 200, "%lu\n", time);
	write(fd_data,buffer,strlen(buffer));
	close(fd);
	fd = -1;
}

/**
 * Get time when we read
*/
void get_read_time(int node_r, int mode)
{
	set_core(1);
	empty_caches();
	load_memory();
	
	pid_t f = fork();
	if (f < 0)
		error("fork");
	if (f == 0) { // Child
		if (config != 1)
			insert_pid_ioctl(getpid());
		set_core(node_r);
		char *buffer = numa_alloc_onnode(file_size, node_r);
		memset(buffer, 0, file_size);

		if (mode)
			read_file(buffer);
		else
			read_file_alea(buffer);
		numa_free(buffer, file_size);
		exit(EXIT_SUCCESS);
	}
	wait(NULL);
}

/**
 * while true read
*/
void loop_reading(char *buf) {
	fd = open(file_path, O_RDONLY);
	if (fd < 0)
		error("open read file alea");
	int i = 100;
	while(i--) {
		// Read randomly in the file
		for(int i = 0; i < 10000; i++) {
			// Choose a location randomly within the file
			int offset = rand() % file_size;
			// int read_len = rand() % (file_size - off_end);
			lseek(fd, offset, SEEK_SET);
			read(fd, buf, 0x2000);
		}
	}
}

/**
 * get time while n process read the file
*/
void get_read_time_proc(int node_r, int mode, int n)
{
	set_core(1);
	empty_caches();
	load_memory();
	
	for(int i = 0; i < n; i++) {
		pid_t f = fork();
		if (f < 0)
			error("fork");
		if (f == 0) { // Child
			set_core(node_r);
			char *buffer = numa_alloc_onnode(file_size, node_r);
			memset(buffer, 0, file_size);
			loop_reading(buffer);
			exit(EXIT_SUCCESS);
		}
	}

	pid_t f = fork();
	if (f < 0)
		error("fork");
	if (f == 0) { // Child
		if (config != 1)
			insert_pid_ioctl(getpid());
		set_core(node_r);
		char *buffer = numa_alloc_onnode(file_size, node_r);
		memset(buffer, 0, file_size);

		if (mode)
			read_file(buffer);
		else
			read_file_alea(buffer);
		numa_free(buffer, file_size);
		exit(EXIT_SUCCESS);
	}
	for (int i = 0; i < n; i++)
		wait(NULL);
}

/**
 * Load n file in the memory 
 * Then read a file
*/
void get_read_time_file(int node_r, int mode, int n)
{
	set_core(1);
	empty_caches();
	if (n != 0)
		load_memory_multiple(n);
	load_memory();

	pid_t f = fork();
	if (f < 0)
		error("fork");
	if (f == 0) { // Child
		if (config != 1)
			insert_pid_ioctl(getpid());
		set_core(node_r);
		char *buffer = numa_alloc_onnode(file_size, node_r);
		memset(buffer, 0, file_size);

		if (mode)
			read_file(buffer);
		else
			read_file_alea(buffer);
		numa_free(buffer, file_size);
		exit(EXIT_SUCCESS);
	}
	wait(NULL);
}

int main(int argc, char *argv[])
{
	// Parameters managment
	if (argc != 3) {
		printf("Usage: %s <num config> <num node>\n", argv[0]);
		return EXIT_FAILURE;
	}

	fd = -1;
	file_path = "res/file";
	config = atoi(argv[1]);
	node = atoi(argv[2]);

	srand(time(NULL));

	// Disable NUMA BALENCING
	if (config != 3)
		write_numa_balencing('0');
	// Enable NUMA BALENCING
	else
		write_numa_balencing('1');

	// Get file_size
	struct stat st;
	if (stat(file_path, &st) != 0)
		error("Error on stat");
	file_size = st.st_size;

	char logname[15] = { 0 };

	printf("------------ Test read on a file ------------\n");
	printf("------------ Test local ------------\n");
	snprintf(logname, 14, "local_seq_%d", config);
	fd_data = open(logname, O_CREAT | O_TRUNC | O_WRONLY, 0777);
	for (int i = 0; i < 100; i++)
		get_read_time(1, 1);

	close(fd_data);
	snprintf(logname, 15, "local_alea_%d", config);
	fd_data = open(logname, O_CREAT | O_TRUNC | O_WRONLY, 0777);
	for (int i = 0; i < 100; i++)
		get_read_time(1, 0);

	close(fd_data);
	printf("---------- Test distant ----------\n");
	snprintf(logname, 15, "distant_seq_%d", config);
	fd_data = open(logname, O_CREAT | O_TRUNC | O_WRONLY, 0777);
	for (int i = 0; i < 100; i++) {
		get_read_time(0, 1);
	}
	close(fd_data);
	snprintf(logname, 15, "distant_alea_%d", config);
	fd_data = open(logname, O_CREAT | O_TRUNC | O_WRONLY, 0777);
	for (int i = 0; i < 100; i++) {
		get_read_time(0, 0);
	}
	close(fd_data);

	printf("------------ Test read with a lot of file ------------\n");
	snprintf(logname, 15, "f0_%d", config);
	fd_data = open(logname, O_CREAT | O_TRUNC | O_WRONLY, 0777);
	for (int i = 0; i < 20; i++)
		get_read_time_file(1, 1, 0);

	snprintf(logname, 15, "f10_%d", config);
	fd_data = open(logname, O_CREAT | O_TRUNC | O_WRONLY, 0777);
	for (int i = 0; i < 20; i++)
		get_read_time_file(1, 1, 10);

	snprintf(logname, 15, "f100_%d", config);
	fd_data = open(logname, O_CREAT | O_TRUNC | O_WRONLY, 0777);
	for (int i = 0; i < 20; i++)
		get_read_time_file(1, 1, 100);
	
	snprintf(logname, 15, "f1000_%d", config);
	fd_data = open(logname, O_CREAT | O_TRUNC | O_WRONLY, 0777);
	for (int i = 0; i < 20; i++)
		get_read_time_file(1, 1, 1000);

	printf("------------ Test read with a lot of process ------------\n");
	snprintf(logname, 15, "p0_%d", config);
	fd_data = open(logname, O_CREAT | O_TRUNC | O_WRONLY, 0777);
	for (int i = 0; i < 20; i++)
		get_read_time_proc(1, 1, 0);
	
	snprintf(logname, 15, "p10_%d", config);
	fd_data = open(logname, O_CREAT | O_TRUNC | O_WRONLY, 0777);
	for (int i = 0; i < 20; i++)
		get_read_time_proc(1, 1, 10);
	close(fd_data);

	snprintf(logname, 15, "p100_%d", config);
	fd_data = open(logname, O_CREAT | O_TRUNC | O_WRONLY, 0777);
	for (int i = 0; i < 20; i++)
		get_read_time_proc(1, 1, 100);
	close(fd_data);

	snprintf(logname, 15, "p1000_%d", config);
	fd_data = open(logname, O_CREAT | O_TRUNC | O_WRONLY, 0777);
	for (int i = 0; i < 20; i++)
		get_read_time_proc(1, 1, 1000);
	close(fd_data);

	return 0;
}