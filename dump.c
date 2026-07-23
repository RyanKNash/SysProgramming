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
	unsigned char buf[Bufsize];

	if(argc != 2) {
		fprintf(stderr, "Usage: dump file\n");
		exit(1);
	}
	fd = open(argv[1], O_RDONLY);
	if(fd < 0) {
		perror("dump");
		exit(1);
	}
	while(1) {
		n = read(fd, buf, Bufsize);
		if(n <= 0)
			break;
		for(i = 0; i < n; i += 16) {
			for(j = 0; j < 16; j++) {
				printf("%02x ", buf[i+j]);
			}
			putchar('\n');
			for(j = 0; j < 16; j++) {
				putchar(' ');
				if(buf[i+j] >= ' ' && buf[i+j] <= '~')
					putchar(buf[i+j]);
				else
					putchar(' ');
				putchar(' ');
			}
			putchar('\n');
		}
	}
	close(fd);
}