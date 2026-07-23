#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct Client Client;
struct Client {
	int fd;
	char name[16];
	Client *prev;
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
		exit(1);
	}
	listen(sockfd, 5);
	while(1) {
		clientfd = accept(sockfd, NULL, NULL);
		newclient = malloc(sizeof(Client));
		newclient->fd = clientfd;
		pthread_mutex_lock(&clientlock);
		newclient->prev = NULL;
		newclient->next = clist;
		if(clist != NULL)
			clist->prev = newclient;
		clist = newclient;
		pthread_mutex_unlock(&clientlock);
fprintf(stderr, "Create client %d\n", clientfd);
		pthread_create(&tid, NULL, client, newclient);
	}
}

void *
client(void *p) {
	Client *c = p;
	Client *q;
	int n, prefix;
	char buf[1024];

	n = read(c->fd, buf, 1024);
	if(buf[n-1] == '\n') {
		n--;
	}
	buf[n] = 0;
	buf[15] = 0;
	strcpy(c->name, buf);
	prefix = strlen(buf);
	buf[prefix++] = '>';
	buf[prefix++] = ' ';
	while(1) {
		n = read(c->fd, buf + prefix, 1024 - prefix);
		if(n <= 0) {
			pthread_mutex_lock(&clientlock);
			if(c->prev == NULL)
				clist = c->next;
			else
				c->prev->next = c->next;
			if(c->next != NULL)
				c->next->prev = c->prev;
			pthread_mutex_unlock(&clientlock);
fprintf(stderr, "exiting %d\n", c->fd);
			pthread_exit(NULL);
		}
		pthread_mutex_lock(&clientlock);
		for(q = clist; q != NULL; q = q->next) {
			write(q->fd, buf, n + prefix);
		}
		pthread_mutex_unlock(&clientlock);
	}
}