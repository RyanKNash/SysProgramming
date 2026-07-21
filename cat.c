#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	BUFSZ = 1024,
};

void cpyfile(int);

int
main(int argc, char *argv[])
{
	int i, fd;

	if(argc == 1) {
		cpyfile(0);
	}
	for(i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-") != 0) {
			fd = open(argv[i], O_RDONLY);
			if(fd < 0) {
				perror("open");
			}
			else {
				cpyfile(fd);
				close(fd);
			}
		}
		else {
			cpyfile(0);
		}
	}
	exit(0);
}

void
cpyfile(int fd)
{
	int n;
	char buf[BUFSZ];

	while((n = read(fd, buf, BUFSZ)) > 0) {
		write(1, buf, n);
	}
	if(n < 0) {
		perror("cpyfile");
	}
}