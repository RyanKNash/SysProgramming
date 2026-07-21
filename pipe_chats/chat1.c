#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

void dochild(int);
void doparent(int);

int
main(int argc, char *argv[]) {
	int who;
	int pid;
	int ppid;

	who = atoi(argv[1]);
	ppid = getpid();
	pid = fork();
	if(pid == 0) {
		dochild(who);
		kill(ppid, SIGINT);
	}
	else {
		doparent(who);
		kill(pid, SIGINT);
	}
}

void
doparent(int who) {
	int outfd;
	int n;
	char buf[128];
	char outfn[10];

	sprintf(outfn, "chatpipe%d", 1 - who);
	outfd = open(outfn, O_WRONLY);
	while(1) {
		n = read(0, buf, 128);
		if(n <= 0)
			return;
		if(buf[0] == '!') {
			buf[strlen(buf)-1] = '\0';
			if(fork() == 0) {
				execlp(buf+1, buf+1, NULL);
			}
			else {
				wait(NULL);
			}
		}
		else {
			write(outfd, buf, n);
		}
	}
}

void
dochild(int who) {
	int infd;
	int n;
	char buf[128];
	char infn[10];

	sprintf(infn, "chatpipe%d", who);
	infd = open(infn, O_RDONLY);
	while(1) {
		n = read(infd, buf, 128);
		if(n <= 0)
			return;
		write(1, buf, n);
	}
}