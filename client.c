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
	char buf[256];
	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	/* 
	   Структрура существует с Linux 2.2, с Linux 1.2 существует
	   ip_mreq, которая совместима с этой, но не включает ifindex.
	   
	 * imr_multiaddr - адрес multicast  группы
	 * imr_address   - адрес локального интерфейса
	 * imr_ifindex   - индекс интерфейса, к которому нужно
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
	socklen_t addr_size = sizeof(addr);
	bind(sock, &addr, sizeof(addr));
	recvfrom(sock, buf, 256, 0, &addr, &addr_size);
	printf("%s\n", buf);
	return 0;
}