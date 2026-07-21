#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

int
main() {
	int fd;
	struct sockaddr_in remote;
	struct hostent *remaddr;
	char buf[100];
	int n;

	fd = socket(AF_INET, SOCK_STREAM, 0);
printf("Got socket\n");
	remaddr = gethostbyname("tux3.cs.drexel.edu");
	if(remaddr == NULL) {
		perror("gethostbyname");
		return 0;
	}
printf("Got address: %x\n", *(remaddr->h_addr));
	memmove(&remote.sin_addr.s_addr, remaddr->h_addr, remaddr->h_length);
	remote.sin_family = AF_INET;
	remote.sin_port = htons(7);
	if(connect(fd, (struct sockaddr *)&remote, sizeof(struct sockaddr_in)) < 0) {
		perror("connect");
		return 0;
	}
printf("Got connection\n");

	write(fd, "Hello", 5);
	n = read(fd, buf, 100);
	write(1, "Got: ", 5);
	write(1, buf, n);
}
