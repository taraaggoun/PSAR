#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/**
 * Load files in mémory
 * While loop on read the first file
*/


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
	lseek(fd, 0, SEEK_SET);
	// Read all the page of the file
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
void read_file(int fd) 
{
	char buf;
	// Get end of file
	int end = lseek(fd, 0, SEEK_END);

	// Read randomly in the file
	while(1) {
		int offset = rand() % end;
		lseek(fd, offset, SEEK_SET);
		int ret = read(fd, &buf, 1);
		if (ret == -1) {
			printf("Error read\n");
			exit(EXIT_FAILURE);
		}
	}
}

int main(int argc, char *argv[])
{
	srand(getpid());
	if (argc < 2) {
		printf("Please Provide at least one argument (file name)\n");
		return 1;
	}
	
	int *fds = calloc(argc - 1, sizeof(int));

	printf("my pid : %d\n", getpid());
	for (int i = 1; i < argc; i++) {
		fds[i - 1] = load_memory(argv[i]);
		if (fds[i - 1] == -1)
			return 1;
		printf("fd%d: %d\n", i, fds[i - 1]);
	}

	read_file(fds[0]);
	
	// Close files
	for (int i = 0; i < argc - 1; i++)
		close(fds[i]);

	free(fds);
	return 0;
}