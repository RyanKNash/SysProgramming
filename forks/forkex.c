#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

void runext(char *, int);
void xfer(int, int, int);

int
main(int argc, char *argv[]) {
	int sendfd, recvfd, pid, ppid;
	
	if(strcmp(argv[1], "Alice") == 0) {
    		sendfd = open("Alicepipe", O_WRONLY);
		recvfd = open("Bobpipe", O_RDONLY);
	}
	else {
		recvfd = open("Alicepipe", O_RDONLY);
		sendfd = open("Bobpipe", O_WRONLY);
	}

	ppid = getpid();	
	pid = fork();
	if(pid == 0) {
		xfer(recvfd, 1, ppid);
	}
	else {
		xfer(0, sendfd, pid);
	}
}

void
xfer(int infd, int outfd, int other) {
	char buf[128];
	int n;
	
	while(1) {
		n = read(infd, buf, 128);
		if(n <= 0) {
			kill(other, SIGINT);
			exit(0);
		}
		if(buf[0] == '>') {
			runext(buf, outfd);
		}
		else {
			write(outfd, buf, n);
		}
	}
}

void
runext(char *cmd, int fd) {
	int pid;
	char *p;

	pid = fork();
	if(pid == 0) {
		close(1);
		dup(fd);
		close(fd);
		p = strchr(cmd, '\n');
		*p = '\0';
		execlp(cmd + 1, cmd + 1, NULL);
		fprintf(stderr, "Cannot run %s\n", cmd + 1);
		exit(1);
	}
	else {
		wait(NULL);
	}
}