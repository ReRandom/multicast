all: server client
server: server.c
	gcc server.c -o server -g -Wall -lpthread
client: client.c
	gcc client.c -o client -g -Wall -lpthread
clear:
	rm server client