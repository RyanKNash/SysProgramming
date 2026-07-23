#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

enum {
	Bufsize = 1024,
};

int
main(int argc, char *argv[]) {
	int fd, n, i, j;
	char *p;
	char buf[Bufsize];
	char line[80];

	if(argc != 2) {
		fprintf(stderr, "Usage: xdump file\n");
		exit(1);
	}
	fd = open(argv[1], O_RDONLY);
	if(fd < 0) {
		perror("xdump");
		exit(1);
	}
	while(1) {
		n = read(fd, buf, Bufsize);
		if(n < 0) {
			perror("read");
			exit(1);
		}
		if(n == 0)
			break;
		for(i = 0; i < n; i += 16) {
			p = line;
			for(j = 0; j < 16; j++) {
				if(i+j >= n) {
					*(p++) = ' ';
					*(p++) = ' ';
				}
				else {
					*(p++) = "0123456789abcdef"[(buf[i+j]>>4)&0xf];
					*(p++) = "0123456789abcdef"[buf[i+j] & 0xf];
				}
				*(p++) = ' ';
			}
			*(p++) = ' ';
			*(p++) = ' ';
			for(j = 0; j < 16; j++) {
				if(i + j >= n)
					*(p++) = ' ';
				else if(buf[i+j] >= ' ' && buf[i+j] <= '~')
					*(p++) = buf[i+j];
				else
					*(p++) = '.';
			}
			*(p++) = '\n';
			write(1, line, p - line);
		}
	}

	close(fd);
}