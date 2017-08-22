#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>

#define ADDR (224 << (3*8)) + 1

void* reciver(void* arg)
{
	char* full_message = NULL;
	size_t size_message = 0;

	char buf[256];

	while(1)
	{
		//Неблокируемое ожидание входящего потока
		int sel_ret = 0;
		while(sel_ret == 0)
		{
			fd_set set;
			struct timeval tv;
			FD_ZERO(&set);
			FD_SET(*(((int**)arg)[0]), &set);
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			sel_ret = select(*(((int**)arg)[0])+1, &set, NULL, NULL, &tv);
			//<возможное прерывание>
		}
		if(sel_ret == -1)
		{
			fprintf(stderr, "error: select\n");
			break;
		}
		socklen_t len = sizeof(struct sockaddr_in);
		ssize_t bytes_read = recvfrom(*(((int**)arg)[0]), buf, 256, 0,
				((struct sockaddr_in**)arg)[1], &len);
		if(bytes_read <= 0)
		{
			printf("Соединение разорвано\n");
			if(full_message != NULL)
			free(full_message);
			break;
		}


		if(full_message != NULL)
		{
			char* reallocated_memory = (char*)realloc(full_message,
					sizeof(char)*(size_message+bytes_read));
			if(reallocated_memory == NULL)
				fprintf(stderr, "Failed to allocate memory\n");
			else
			{
				full_message = reallocated_memory;
				memcpy(full_message+size_message, buf, (size_t)bytes_read);
				size_message += (size_t)bytes_read;
			}
		}
		else
		{
			full_message = (char*)malloc(sizeof(char)*bytes_read);
			if(full_message == NULL)
				fprintf(stderr, "Failed to allocate memory\n");
			size_message = bytes_read;
			memcpy(full_message, buf, (size_t)bytes_read);
		}
		if(full_message[size_message-1] == '\0')
		{
			printf("%s\n", full_message);
			free(full_message);
			full_message = NULL;
			size_message = 0;
		}
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
	//Помечаем сокет неблокируемым
	if(fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
	perror("fcntl");

	//Говорим, что хотим multicast трафик с заданого адреса
		/*
		   Структрура существует с Linux 2.2, с Linux 1.2 существует
		   ip_mreq, которая совместима с этой, но не включает ifindex.
		 * imr_multiaddr - адрес multicast  группы
		 * imr_address	 - адрес локального интерфейса
		 * imr_ifindex	 - индекс интерфейса, к которому нужно
						   присоединить группу, или 0 для любого
						   интерфейса. (?)
		*/
		struct ip_mreqn mr;
		struct in_addr multicast_addr = { htonl(ADDR) };
		mr.imr_multiaddr = multicast_addr;
		struct in_addr my_addr = { htonl(INADDR_ANY) };
		mr.imr_address = my_addr;
		mr.imr_ifindex = 0;
	setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mr, sizeof(mr));
	struct sockaddr_in addr = {AF_INET, htons(7777), htonl(INADDR_ANY)};
	if(bind(sock, &addr, sizeof(addr)) == -1)
	{
		perror("bind");
		return 1;
	}

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
	if(pthread_create(&thread, NULL, reciver, arg) != 0)
	{
		fprintf(stderr, "Failed to create thread");
		free(arg);
		if(close(sock) == -1)
			perror("close");
		return 1;
	}

	if(pthread_join(thread, NULL) != 0)
	{
		fprintf(stderr, "Failed to join thread");
		free(arg);
		if(close(sock) == -1)
			perror("close");
		return 1;
	}
	free(arg);
	if(close(sock) == -1)
		perror("close");
	return 0;
}
