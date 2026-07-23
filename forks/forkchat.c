#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

void doxfer(int, int, int);
void runext(int, char *, int);

int
main(int argc, char *argv[]) {
	int sendfd, recvfd;
	int childpid, parentpid;

	if(strcmp(argv[1], "Alice") == 0) {
		sendfd = open("Alicepipe", O_WRONLY);
		recvfd = open("Bobpipe", O_RDONLY);
	}
	else {
		recvfd = open("Alicepipe", O_RDONLY);
		sendfd = open("Bobpipe", O_WRONLY);
	}
	parentpid = getpid();	
	childpid = fork();
	if(childpid == 0) {
		doxfer(recvfd, 1, parentpid);
	}
	else {
		doxfer(0, sendfd, childpid);
	}
}

void
doxfer(int infd, int outfd, int otherpid) {
	char buf[128];
	int n;

	while(1) {
		n = read(infd, buf, 127);
		if(n <= 0) {
			kill(otherpid, SIGINT);
			exit(0);
		}
		if(buf[0] == '>') {
			runext(outfd, buf + 1, n - 1);
		}
		else {
			write(outfd, buf, n);
		}
	}
}

void
runext(int fd, char *cmd, int n) {
	int pid;
	char *p;

	cmd[n] = '\0';
	p = strchr(cmd, '\n');
	if(p != NULL)
		*p = '\0';
	pid = fork();
	if(pid == 0) {
		close(1);
		dup(fd);
		close(fd);
		execlp(cmd, cmd, NULL);
		fprintf(stderr, "Cannot run %s\n", cmd);
		exit(1);
	}
	else {
		wait(NULL);
	}
}
