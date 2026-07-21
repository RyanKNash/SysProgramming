#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

long count;
pthread_mutex_t lock;

void *counter(void *);

int
main()
{
	pthread_t tid1, tid2;

	pthread_mutex_init(&lock, NULL);

	if(pthread_create(&tid1, NULL, counter, NULL) != 0) {
		perror("pthread_create");
		exit(1);
	}
	if(pthread_create(&tid2, NULL, counter, NULL) != 0) {
		perror("pthread_create");
		exit(1);
	}

	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);

	printf("%ld\n", count);
}

void *
counter(void *dummy)
{
	long i;
	long temp;

	for(i = 0; i < 12000000; i++) {
		pthread_mutex_lock(&lock);
		temp = count;
		temp = temp + 1;
		count = temp;
		pthread_mutex_unlock(&lock);
	}
	pthread_exit(NULL);
}