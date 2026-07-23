#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void dochild(int);
void doparent(int);

int
main(int argc, char *argv[])
{
	int which;
	int chpid;

	if(argc != 2) {
		fprintf(stderr, "Usage: chat which\n");
		exit(1);
	}
	which = atoi(argv[1]);

	chpid = fork();
	if(chpid == 0) {
		dochild(which);
	}
	else {
		doparent(which);
	}
}

void
runcommand(int fdw, char *cmd)
{
	int status, i, n;
	char *toks[32];

	n = strlen(cmd);
	if(cmd[n-1] == '\n')
		cmd[n-1] = '\0';
	if(fork() == 0) {
		toks[0] = strtok(cmd, " \t\n");
		for(i = 1; i < 32; i++) {
			toks[i] = strtok(NULL, " \t\n");
			if(toks[i] == NULL)
				break;
		}
		if(fdw != 0) {
			close(1);
			dup(fdw);
		}
		execvp(toks[0], toks);
		perror("exec");
		exit(3);
	}
	else {
		wait(&status);
	}
}

void
doparent(int which)
{
	int fdw, n;
	char buf[128];

	if(which == 1) {
		fdw = open("chatpipe1", O_WRONLY);
	}
	else {
		fdw = open("chatpipe2", O_WRONLY);
	}
	if(fdw < 0) {
		perror("pipes");
		exit(2);
	}

	while(1) {
		n = read(0, buf, 127);
		if(n <= 0)
			break;
		buf[n] = '\0';
		switch(buf[0]) {
		case '!':
			runcommand(0, buf+1);
			break;
		case '>':
			runcommand(fdw, buf+1);
			break;
		default:
			write(fdw, buf, n);
			break;
		}
	}
	exit(0);
}

void
dochild(int which)
{
	int fdr, n;
	char buf[128];

	if(which == 1) {
		fdr = open("chatpipe2", O_RDONLY);
	}
	else {
		fdr = open("chatpipe1", O_RDONLY);
	}
	if(fdr < 0) {
		perror("pipes");
		exit(2);
	}

	while(1) {
		n = read(fdr, buf, 127);
		buf[n] = '\0';
		if(n <= 0)
			break;
		write(1, buf, n);
	}
	exit(0);
}