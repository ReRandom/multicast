#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define ADDR (224 << (3*8)) + 7
#define MESSAGE "Hello from server!"
#define MESSAGE_LEN (strlen(MESSAGE)+1)*sizeof(char)

void* sender(void* arg)
{
	char* msg = (char*) malloc(MESSAGE_LEN);
	if(msg == NULL)
		fprintf(stderr, "Failed to allocate memory\n");
	msg = MESSAGE;
	while(1)
	{
		if(sendto(*(((int**)arg)[0]), msg, MESSAGE_LEN, 0,
				  ((struct sockaddr_in**)arg)[1],
				  sizeof(struct sockaddr_in)) == -1)
			perror("send");
		if(sleep(2) != 0)
			fprintf(stderr, "sleep\n");
	}
	return NULL;
}

int main(int argc, char* argv[])
{
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
	{
		perror("socket");
		return 1;
	}
	struct sockaddr_in addr = { AF_INET, htons(7777), htonl(ADDR)};

	void* arg = malloc(sizeof(void*)*2);
	if(arg == NULL)
	{
		fprintf(stderr, "Failed to allocate memory\n");
		if(close(sock) == -1)
			perror("close");
		return 1;
	}
	((void**)arg)[0] = &sock;
	((void**)arg)[1] = &addr;
	pthread_t thread;
	if(pthread_create(&thread, NULL, sender, arg) != 0)
	{
		fprintf(stderr, "Failed to create thread\n");
		free(arg);
		if(close(sock) == -1)
			perror("close");
		return 1;
	}

	if(pthread_join(thread, NULL) != 0)
	{
		fprintf(stderr, "Failed to join thread\n");
		free(arg);
		if(close(sock) == -1)
			perror("close");
		return 1;
	}
	free(arg);
	if(close(sock) == -1)
	{
		perror("close");
		return 1;
	}
	return 0;
}
