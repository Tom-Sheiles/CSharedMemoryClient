all: server client

server: server1.c
	gcc -o server server1.c
	
client: client.c
	gcc -o client client.c