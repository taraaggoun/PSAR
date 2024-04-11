#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>

/**
 * Take one argument a file
 * and while loop on reading it
*/

int main(int argc, char *argv[]) {
	if (argc =! 2) {
		printf("Error, bad number of argument \n");
		return 1;
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		printf("Erreur open\n");
		return 1;
	}
	int i = 0;
	char c;
	int end =  lseek(fd, 0, SEEK_END);
	while (1) {
		int offset = rand() % end;
		lseek(fd, offset, SEEK_SET);
		if (read(fd, &c,  1) < 0) {
			printf("Erreur read\n");
			return -1;
		}
	}
	return 0;
}