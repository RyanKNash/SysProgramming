#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef struct Client Client;
struct Client {
	int fd;
	char name[16];
	Client *next;
};

void *client(void *);

pthread_mutex_t clientlock;
Client *clist;

int
main() {
	int sockfd;
	int clientfd;
	pthread_t tid;
	struct sockaddr_in local;
	Client *newclient;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	local.sin_family = AF_INET;
	local.sin_port = htons(2024);
	local.sin_addr.s_addr = INADDR_ANY;
	if(bind(sockfd, (struct sockaddr *)&local, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		return 1;
	}
	listen(sockfd, 5);
	while(1) {
		clientfd = accept(sockfd, NULL, NULL);
fprintf(stderr, "creating child\n");
		newclient = malloc(sizeof(Client));
		newclient->fd = clientfd;
		pthread_mutex_lock(&clientlock);
		newclient->next = clist;
		clist = newclient;
		pthread_mutex_unlock(&clientlock);
		pthread_create(&tid, NULL, client, newclient);
	}
}

void *
client(void *p) {
	Client *c = p;
	Client *q, *qp;
	int n, prefix;
	char buf[1024], buf2[1042];

	n = read(c->fd, buf, 1024);
	buf[n] = 0;
	if(buf[n-1] == '\n') {
		buf[n-1] = 0;
		n--;
	}
	strncpy(c->name, buf, 15);
	c->name[15] = 0;
	sprintf(buf2, "%s> ", c->name);
	prefix = strlen(buf2);
	while(1) {
		n = read(c->fd, buf, 1024);
		if(n <= 0) {
			pthread_mutex_lock(&clientlock);
			qp = NULL;
			q = clist;
			while(q != NULL && q != c) {
				qp = q;
				q = q->next;
			}
			if(qp == NULL)
				clist = q->next;
			else
				qp->next = q->next;
			pthread_mutex_unlock(&clientlock);
			free(q);
fprintf(stderr, "Exiting\n");
			pthread_exit(NULL);
		}
		memmove(buf2 + prefix, buf, n);
		pthread_mutex_lock(&clientlock);
		for(q = clist; q != NULL; q = q->next) {
			write(q->fd, buf2, n + prefix);
		}
		pthread_mutex_unlock(&clientlock);
	}
}