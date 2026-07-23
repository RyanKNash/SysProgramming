#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

void dochild(int);
void doparent(int);

int
main(int argc, char *argv[]) {
	int pid;
	int whoami;

	whoami = atoi(argv[1]);
	pid = fork();
	if(pid == 0)
		dochild(whoami);
	else
		doparent(whoami);
}

void
dochild(int who) {
	int n;
	int fd;
	char fn[10];
	char buf[1024];

	sprintf(fn, "c%d", who);
	fd = open(fn, O_RDONLY);
	while(1) {
		n = read(fd, buf, 1024);
		if(n <= 0)
			break;
		write(1, buf, n);
	}
}

void
runprog(char *cmd, int out) {
	char *p;
	char *argv[2];
	int pid;

	p = strchr(cmd, '\n');
	*p = 0;
	argv[0] = cmd;
	argv[1] = NULL;
	pid = fork();
	if(pid == 0) {
		if(out != 1) {
			close(1);
			dup(out);
			close(out);
		}
		execvp(argv[0], argv);
		perror("exec");
		exit(0);
	}
	else {
		wait(NULL);
	}
}

void
doparent(int who) {
	int other, n;
	int fd;
	char fn[10];
	char buf[1024];

	other = 1 - who;
	sprintf(fn, "c%d", other);
	fd = open(fn, O_WRONLY);
	while(1) {
		n = read(0, buf, 1024);
		if(n <= 0)
			break;
		if(buf[0] == '!') {
			runprog(buf + 1, 1);
		}
		else if(buf[0] == '>') {
			runprog(buf + 1, fd);
		}
		else {
			write(fd, buf, n);
		}
	}
}