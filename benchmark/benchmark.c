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
}

// void load_memory_multiple(int n) {
// 	int fds[n];
// 	for (int i = 0; i < n; i++) {
// 		char pathname[10] = { 0 };
// 		snprintf(pathname, 10, "file%d", i);
// 		fds[i] = open(pathname, O_RDONLY);
// 		if (fds[i] < 0)
// 			error("open fds");
// 	}
// 	// Get EOF
// 	off_t end = lseek(fds[0], 0, SEEK_END);
// 	lseek(fds[0], 0, SEEK_SET);

// 	char buf;
// 	// Sequential reading of the file to map it into memory
// 	for (int i = 0; i < n; i++) {
// 		while (1) {
// 			int r = read(fd, &buf, 1);
// 			if (r == 0)
// 				break;
// 			if (r == -1)
// 				error("read in load memory with a read");

// 			// Move the pointer to the next page
// 			off_t cur =  lseek(fd, 1 << 12, SEEK_CUR);
// 			if (cur < 0)
// 				error("lseek to go to next page");

// 			// Return NULL if we reach the end of the file
// 			if (cur == end)
// 				break;
// 		}
// 	}
// 	close(fd);
// 	fd = -1;
// }

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
		// insert_pid_ioctl(getpid());
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

// /**
//  * while true read
// */
// void loop_reading(char *buf) {
// 	fd = open(file_path, O_RDONLY);
// 	if (fd < 0)
// 		error("open read file alea");
// 	int i = 100;
// 	while(i--) {
// 		// Read randomly in the file
// 		for(int i = 0; i < 10000; i++) {
// 			// Choose a location randomly within the file
// 			int offset = rand() % file_size;
// 			// int read_len = rand() % (file_size - off_end);
// 			lseek(fd, offset, SEEK_SET);
// 			read(fd, buf, 0x2000);
// 		}
// 	}
// }

// /**
//  * get time while n process read the file
// */
// void get_read_time_proc(int node_r, int mode, int n)
// {
// 	set_core(1);
// 	empty_caches();
// 	load_memory_read();
	
// 	for(int i = 0; i < n; i++) {
// 		pid_t f = fork();
// 		if (f < 0)
// 			error("fork");
// 		if (f == 0) { // Child
// 			// insert_pid_ioctl(getpid());
// 			set_core(node_r);
// 			char *buffer = numa_alloc_onnode(file_size, node_r);
// 			memset(buffer, 0, file_size);
// 			loop_reading(buffer);
// 			exit(EXIT_SUCCESS);
// 		}
// 	}

// 	pid_t f = fork();
// 	if (f < 0)
// 		error("fork");
// 	if (f == 0) { // Child
// 		sleep(1);
// 		set_core(node_r);
// 		char *buffer = numa_alloc_onnode(file_size, node_r);
// 		memset(buffer, 0, file_size);

// 		if (mode)
// 			read_file(buffer);
// 		else
// 			read_file_alea(buffer);
// 		print_node();
// 		numa_free(buffer, file_size);
// 		exit(EXIT_SUCCESS);
// 	}
// 	for (int i = 0; i < n; i++)
// 		wait(NULL);
// }

// void get_read_time_file(int node_r, int mode, int n)
// {

// }

int main(int argc, char *argv[])
{
	// Parameters managment
	if (argc != 4) {
		printf("Usage: %s <file_path> <num config> <num node>\n", argv[0]);
		return EXIT_FAILURE;
	}

	fd = -1;
	file_path = argv[1];
	config = atoi(argv[2]);
	node = atoi(argv[3]);

	srand(time(NULL));

	// Get file_size
	struct stat st;
	if (stat(file_path, &st) != 0)
		error("Error on stat");
	file_size = st.st_size;


	printf("------------ Test read on a file ------------\n");
	printf("------------ Test local ------------\n");
	fd_data = open("test_local_seq", O_CREAT|O_TRUNC|O_WRONLY,0777);
	for (int i = 0; i < 100; i++)
		get_read_time(1, 1);

	close(fd_data);
	fd_data = open("test_local_alea", O_CREAT|O_TRUNC|O_WRONLY,0777);
	for (int i = 0; i < 100; i++)
		get_read_time(1, 0);

	close(fd_data);
	printf("---------- Test distant ----------\n");
	fd_data = open("test_distant_seq", O_CREAT|O_TRUNC|O_WRONLY,0777);
	for (int i = 0; i < 100; i++) {
		get_read_time(0, 1);
	}
	close(fd_data);
	fd_data = open("test_distant_alea", O_CREAT|O_TRUNC|O_WRONLY,0777);
	for (int i = 0; i < 100; i++) {
		get_read_time(0, 0);
	}
	close(fd_data);

	/* printf("------------ Test lot of files ------------\n");
	printf("------------ Test local ------------\n");
	fd_data = open("test_local_seq_p10_1", O_CREAT|O_TRUNC|O_WRONLY,0777);
	for (int i = 0; i < 20; i++)
		get_read_time_proc(1, 1, 10);
	close(fd_data);
	fd_data = open("test_local_alea_p10_1", O_CREAT|O_TRUNC|O_WRONLY,0777);
	for (int i = 0; i < 20; i++)
		get_read_time_proc(1, 0, 10);
	close(fd_data);
	// fd_data = open("test_local_seq_p100_1", O_CREAT|O_TRUNC|O_WRONLY,0777);
	// for (int i = 0; i < 20; i++)
	// 	get_read_time_proc(1, 1, 100);
	// close(fd_data);
	// fd_data = open("test_local_alea_p100_1", O_CREAT|O_TRUNC|O_WRONLY,0777);
	// for (int i = 0; i < 20; i++)
	// 	get_read_time_proc(1, 0, 100);
	// close(fd_data);
	// fd_data = open("test_local_seq_p1000_1", O_CREAT|O_TRUNC|O_WRONLY,0777);
	// for (int i = 0; i < 20; i++)
	// 	get_read_time_proc(1, 1, 1000);
	// close(fd_data);
	// fd_data = open("test_local_alea_p1000_1", O_CREAT|O_TRUNC|O_WRONLY,0777);
	// for (int i = 0; i < 20; i++)
	// 	get_read_time_proc(1, 0, 1000);
	close(fd_data);
	printf("---------- Test distant ----------\n");
	fd_data = open("test_distant_seq_p10_1", O_CREAT|O_TRUNC|O_WRONLY,0777);
	for (int i = 0; i < 20; i++)
		get_read_time_proc(0, 1, 10);
	close(fd_data);
	fd_data = open("test_distant_alea_p10_1", O_CREAT|O_TRUNC|O_WRONLY,0777);
	for (int i = 0; i < 20; i++)
		get_read_time_proc(0, 0, 10);
	close(fd_data);
	// fd_data = open("test_distant_seq_p100_1", O_CREAT|O_TRUNC|O_WRONLY,0777);
	// for (int i = 0; i < 20; i++)
	// 	get_read_time_proc(0, 1, 100);
	// close(fd_data);
	// fd_data = open("test_distant_alea_p100_1", O_CREAT|O_TRUNC|O_WRONLY,0777);
	// for (int i = 0; i < 20; i++)
	// 	get_read_time_proc(0, 0, 100);
	// close(fd_data);
	// fd_data = open("test_distant_seq_p1000_1", O_CREAT|O_TRUNC|O_WRONLY,0777);
	// for (int i = 0; i < 20; i++)
	// 	get_read_time_proc(0, 1, 1000);
	// close(fd_data);
	// fd_data = open("test_distant_alea_p1000_1", O_CREAT|O_TRUNC|O_WRONLY,0777);
	// for (int i = 0; i < 20; i++)
	// 	get_read_time_proc(0, 0, 1000);
	// close(fd_data);
 */	return 0;
}