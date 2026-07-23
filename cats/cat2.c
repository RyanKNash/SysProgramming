#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

void cat(int);

int
main(int argc, char *argv[]) {
	int fd;
	int i;

	if(argc == 1) {
		cat(0);
		exit(0);
	}
	for(i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-") == 0)
			cat(0);
		else {
			fd = open(argv[i], O_RDONLY);
			cat(fd);
			close(fd);
		}
	}
}

void
cat(int fd) {
	char buf[32];
	int n;

	while(1) {
		n = read(fd, buf, 32);
		if(n == 0)
			break;
		write(1, buf, n);
	}
}
