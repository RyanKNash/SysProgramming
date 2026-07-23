#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>

typedef struct Client Client;

struct Client {
	int sock;
	pthread_t tid;
	char name[16];
	Client *next, *prev;
};

void *client(void *);
void cdel(Client *);
void *ctl(void *);

Client *chead;
pthread_mutex_t client_lock;

int
main()
{
	int sock, n;
	pthread_t tid;
	struct sockaddr_in listener;
	Client *c, *c2;

	pthread_mutex_init(&client_lock, NULL);
	pthread_create(&tid, NULL, ctl, NULL);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	listener.sin_family = AF_INET;
	listener.sin_port = htons(2023);
	listener.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sock, (struct sockaddr *)&listener, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		exit(1);
	}
	while(1) {
		if(listen(sock, 5) < 0) {
			perror("listen");
			exit(2);
		}
		c = malloc(sizeof(Client));
		c->sock = accept(sock, NULL, NULL);
		if(c->sock < 0) {
			perror("accept");
			free(c);
		}
		else {
			pthread_mutex_lock(&client_lock);
			for(c2 = chead; c2 != NULL && c2->sock != c->sock; c2 = c2->next) ;
			if(c2 != NULL) {
				pthread_cancel(c2->tid);
				cdel(c2);
			}
			pthread_mutex_unlock(&client_lock);
			pthread_create(&tid, NULL, client, c);
		}
	}
}

void *
ctl(void *a)
{
	Client *p;
	FILE *fp;
	int fd, n;
	char buf[128];
	pthread_t tid;

	tid = pthread_self();
	pthread_detach(tid);
	while(1) {
		fd = open("chatctl", O_RDONLY);
		if(fd <= 0) {
			fprintf(stderr, "No control pipe\n");
			pthread_exit(NULL);
		}
		n = read(fd, buf, 128);
		if(n == 0)
			fprintf(stderr, "null read on ctl\n");
		close(fd);
		buf[n] = 0;
		if(strncmp(buf, "shutdown", 8) == 0)
			exit(0);
		if(strncmp(buf, "stat", 4) == 0) {
			fp = fopen("chatstat", "w");
			pthread_mutex_lock(&client_lock);
			for(p = chead; p != NULL; p = p->next)
				fprintf(fp, "%d %s\n", p->sock, p->name);
			pthread_mutex_unlock(&client_lock);
			fclose(fp);
		}
	}
}


void
cadd(Client *c)
{
	pthread_mutex_lock(&client_lock);
	c->prev = NULL;
	if(chead != NULL) {
		chead->prev = c;
	}
	c->next = chead;
	chead = c;
	pthread_mutex_unlock(&client_lock);
}

void
cdel(Client *c)
{
	if(chead == c) {
		chead = c->next;
	}
	if(c->next != NULL) {
		c->next->prev = c->prev;
	}
	if(c->prev != NULL) {
		c->prev->next = c->next;
	}
	free(c);
}

void *
client(void *p)
{
	Client *c = p;
	Client *q, *r;
	char buf[1024];
	int i, n, prefix, nfd;
	int *fds;

	c->tid = pthread_self();
	pthread_detach(c->tid);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &n);
	n = recv(c->sock, buf, 1024, 0);
	if(n <= 0) {
		shutdown(c->sock, SHUT_RDWR);
		free(c);
		pthread_exit(NULL);
	}
	if(n > 15)
		n = 15;
	if(buf[n-1] == '\n')
		n--;
	strncpy(c->name, buf, n);
	c->name[n] = '\0';
	prefix = sprintf(buf, "%s> ", c->name);
	cadd(c);
	while(1) {
		n = recv(c->sock, buf + prefix, 1024 - prefix, 0);
		if(n <= 0) {
			shutdown(c->sock, SHUT_RDWR);
			close(c->sock);
			pthread_mutex_lock(&client_lock);
			cdel(c);
			pthread_mutex_unlock(&client_lock);
			pthread_exit(NULL);
		}
		pthread_mutex_lock(&client_lock);
		for(q = chead, nfd = 0; q != NULL; q = q->next, nfd++) ;
		fds = malloc(nfd * sizeof(int));
		for(q = chead, i = 0; q != NULL; q = q->next, i++)
			fds[i] = q->sock;
		pthread_mutex_unlock(&client_lock);
		for(i = 0; i < nfd; i++) {
			if(fds[i] != c->sock) {
				if(send(fds[i], buf, n + prefix, MSG_NOSIGNAL) <= 0) {
					shutdown(fds[i], SHUT_RDWR);
					pthread_mutex_lock(&client_lock);
					for(q = chead; q != NULL; q = q->next)
						if(q->sock == fds[i])
							break;
					if(q != NULL) {
						pthread_cancel(q->tid);
						close(q->sock);
						cdel(q);
					}
					pthread_mutex_unlock(&client_lock);
				}
			}
		}
		free(fds);
	}
}