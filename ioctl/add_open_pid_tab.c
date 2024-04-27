#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>

#define IOCTL_MAGIC 'N'
#define PIDW _IOW(IOCTL_MAGIC, 0, char*)

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
		return 1;
	}

	int fd = open("/dev/openctl", O_WRONLY);
	if (fd == -1) {
		perror("Error on open device file\n");
		return 1;
	}

	if (ioctl(fd, PIDW, argv[1]) == -1) {
		perror("Error ioctl\n");
		close(fd);
		return 1;
	}

	close(fd);
	return 0;
}