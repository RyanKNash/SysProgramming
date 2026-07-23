#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void runchild(int *);

int
main()
{
	int pfd[2];
	char *msg;

	msg = "Hello World\n";
	pipe(pfd);
	if(fork() == 0) {
		runchild(pfd);
	}
	else {
		close(pfd[0]);
		printf("Sending message to child\n");
		write(pfd[1], msg, strlen(msg));
	}
}

void
runchild(int *pfd)
{
	char buf[80];
	int n;

	close(pfd[1]);
	n = read(pfd[0], buf, 80);
	buf[n] = '\0';
	printf("Got: %s\n", buf);
	exit(0);
}