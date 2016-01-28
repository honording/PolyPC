
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

int main() {
	int fd;
	fd = open("/dev/hapara_test", O_RDWR);
	if (fd == -1) {
		printf("open failed\n\r");
		return 0;
	}
	return 0;
}
