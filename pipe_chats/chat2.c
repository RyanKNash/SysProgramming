#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

void dochild(int);
void doparent(int);

int
main(int argc, char *argv[]) {
	int pid, ppid;
	int who;

	who = atoi(argv[1]);
	ppid = getpid();
	pid = fork();
	if(pid == 0) {
		dochild(who);
		kill(ppid, SIGINT);
	}
	else {
		doparent(who);
	}
}

void
doparent(int who) {
	char buf[128];
	int fd, n;

	sprintf(buf, "chatpipe%d", 1-who);
	fd = open(buf, O_WRONLY);
	while(1) {
		n = read(0, buf, 128);
		if(n <= 0)
			return;
		if(buf[0] == '!') {
			*strchr(buf, '\n') = '\0';
			if(fork() == 0) {
				execlp(buf+1, buf+1, NULL);
			}
			else {
				wait(NULL);
			}
		}
		else {
			write(fd, buf, n);
		}
	}
}

void
dochild(int who) {
	char buf[128];
	int fd, n;

	sprintf(buf, "chatpipe%d", who);
	fd = open(buf, O_RDONLY);
	while(1) {
		n = read(fd, buf, 128);
		if(n <= 0)
			return;
		write(1, buf, n);
	}
}