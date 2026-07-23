#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

void runchild(int, int);
void runparent(int, int, int);

int inpredirproc;
int specialfd1, specialfd2;

int
main(int argc, char *argv[])
{
	int chpid;
	int pfd[2];

	if(argc != 2) {
		fprintf(stderr, "Usage: chat n\n");
		exit(1);
	}

	pipe(pfd);
	chpid = fork();
	if(chpid < 0) {
		perror("fork");
		exit(2);
	}
	if(chpid == 0) {
		close(pfd[1]);
		runchild(atoi(argv[1]), pfd[0]);
	}
	else {
		close(pfd[0]);
		runparent(atoi(argv[1]), chpid, pfd[1]);
	}
	exit(0);
}

int
runcommand(char *buf, int fdw, int fdr)
{
	int i, chpid;
	char *tokens[32];

	chpid = fork();
	if(chpid > 0) {
		return chpid;
	}
	else {
		signal(SIGINT, SIG_DFL);
		tokens[0] = strtok(buf + 1, " \t\n");
		for(i = 1; i < 32; i++) {
			tokens[i] = strtok(NULL, " \t\n");
			if(tokens[i] == NULL)
				break;
		}
		if(fdw != -1) {
			close(1);
			dup(fdw);
		}
		if(fdr != -1) {
			close(0);
			dup(fdr);
		}
		execvp(tokens[0], tokens);
		perror("exec");
		exit(4);
	}
	return(0);
}

void
runparent(int which, int chpid, int pfd)
{
	int fdw, n, status;
	char buf[128];

	if(which == 1) {
		fdw = open("chatpipe1", O_WRONLY);
	}
	else {
		fdw = open("chatpipe2", O_WRONLY);
	}
	if(fdw < 0) {
		perror("parent open");
		exit(3);
	}

	signal(SIGINT, SIG_IGN);

	while((n = read(0, buf, 127)) > 0) {
		buf[n] = '\0';
		switch(buf[0]) {
		case '!':
			runcommand(buf, -1, -1);
			wait(&status);
			break;
		case '>':
			runcommand(buf, fdw, -1);
			wait(&status);
			break;
		case '<':
			if(buf[1] == '$') {
				kill(inpredirproc, SIGINT);
			}
			else {
				kill(chpid, SIGUSR1);
				write(pfd, buf, n+1);
			}
			break;
		default:
			write(fdw, buf, n);
			break;
		}
	}
	close(fdw);
}

void
usr1hdlr(int x)
{
	char buf[128];
	int status;

	read(specialfd1, buf, 128);
	inpredirproc = runcommand(buf, -1, specialfd2);
	wait(&status);
}

void
runchild(int which, int pfd)
{
	int fdr, n;
	char buf[128];

	signal(SIGUSR1, usr1hdlr);
	signal(SIGINT, SIG_IGN);

	if(which == 1) {
		fdr = open("chatpipe2", O_RDONLY);
	}
	else {
		fdr = open("chatpipe1", O_RDONLY);
	}
	if(fdr < 0) {
		perror("child open");
		exit(3);
	}

	specialfd1 = pfd;
	specialfd2 = fdr;
	while((n = read(fdr, buf, 128)) > 0) {
		write(1, ">>> ", 4);
		write(1, buf, n);
	}
	close(fdr);
}