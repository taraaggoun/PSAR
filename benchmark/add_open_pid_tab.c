#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h> // For strlen()

#define IOCTL_MAGIC 'N'
#define PIDW _IOW(IOCTL_MAGIC, 0, char*)

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <data>\n", argv[0]);
        return 1;
    }

    int fd = open("/dev/openctl", O_WRONLY);
    if (fd == -1) {
        perror("Error opening device file\n");
        return 1;
    }

    // Prepare data to be sent to the device driver
    char data[256]; // Assuming maximum length of data is 256
    strncpy(data, argv[1], sizeof(data)); // Copy command-line argument to data buffer

    if (ioctl(fd, PIDW, data) == -1) {
        perror("Error ioctl\n");
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}
