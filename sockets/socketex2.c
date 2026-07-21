#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int
main() {
	int sock;
	struct hostent *remote;
	struct sockaddr_in remaddr;
	char buf[100];
	int n;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	remote = gethostbyname("tux1.cs.drexel.edu");
	if(remote == NULL) {
		perror("gethostbyname");
		return 1;
	}
	memmove(&remaddr.sin_addr.s_addr, remote->h_addr, remote->h_length);
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(10007);
	if(connect(sock, (struct sockaddr *)(&remaddr), sizeof(struct sockaddr_in)) < 0) {
		perror("connect");
		return 1;
	}
	write(sock, "Hello", 5);
	n = read(sock, buf, 100);
	buf[n] = 0;
	printf("Got: %s\n", buf);
}