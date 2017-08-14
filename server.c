#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ADDR (224 << (3*8)) + 1

void* sender(void* arg)
{
	char* msg = (char*) malloc(4*sizeof(char));
	if(msg == NULL)
		fprintf(stderr, "Failed to allocate memory\n");
	msg[3] = '\0';
	msg[0] = 'h';
	msg[1] = 'i';
	msg[2] = '!';
	while(1)
	{
		if(sendto(*(((int**)arg)[0]), msg, sizeof(char)*4, 0, 
				  ((struct sockaddr_in**)arg)[1], 
				  sizeof(struct sockaddr_in)) == -1)
			perror("send");
		if(sleep(2) != 0)
			fprintf(stderr, "sleep\n");
	}
}

int main(int argc, char* argv[])
{
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
		perror("socket");
	struct sockaddr_in addr = { AF_INET, htons(7777), htonl(ADDR)};

	void* arg = malloc(sizeof(void*)*2);
	if(arg == NULL)
		fprintf(stderr, "Failed to allocate memory\n");
	((void**)arg)[0] = &sock;
	((void**)arg)[1] = &addr;
	pthread_t thread;
	if(pthread_create(&thread, NULL, sender, arg) != 0)
		fprintf(stderr, "Failed to create thread\n");

	pthread_join(thread, NULL);
	free(arg);	
	close(sock);
	return 0;
}