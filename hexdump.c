#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

enum {
	Bufsize = 1024,
};

int hexdigit(int);

int
main(int argc, char *argv[]) {
	int fd, n, i, j;
	char *p;
	char buf[Bufsize];
	char line[80];

	if(argc != 2) {
		fprintf(stderr, "Usage: hexdump file\n");
		exit(1);
	}
	fd = open(argv[1], O_RDONLY);
	if(fd < 0) {
		perror("hexdump");
		exit(1);
	}
	while(1) {
		n = read(fd, buf, Bufsize);
		if(n <= 0)
			break;
		for(i = 0; i < n; i += 16) {
			p = line;
			for(j = 0; j < 16; j++) {
				*p = hexdigit(buf[i+j] >> 4);
				p++;
				*p = hexdigit(buf[i+j]);
				p++;
				*p = ' ';
				p++;
				if(buf[i+j] >= ' ' && buf[i+j] <= '~')
					line[50+j] = buf[i+j];
				else
					line[50+j] = '.';
			}
			*(p++) = ' ';
			*(p++) = ' ';
			line[66] = '\n';
			write(1, line, 67);
		}
	}
	close(fd);
}

char *hextab = "0123456789abcdef";

int
hexdigit(int x) {
	return hextab[x & 0x0f];
}