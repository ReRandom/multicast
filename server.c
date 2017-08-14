#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ADDR (224 << (3*8)) + 1

int main(int argc, char* argv[])
{
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in addr = { AF_INET, htons(7777), htonl(ADDR)};

	char* msg = (char*) malloc(4*sizeof(char));
	msg[3] = '\0';
	msg[0] = 'h';
	msg[1] = 'i';
	msg[2] = '!';
	sendto(sock, msg, sizeof(char)*4, 0, &addr, sizeof(addr));
	close(sock);
	return 0;
}