all: server client

server: server1.c
	sudo gcc -o server server1.c
	
client: client.c
	sudo gcc -o client client.c