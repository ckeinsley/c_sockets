all: client server

client: client.c
	gcc client.c -o client

server: server.c server.h
	gcc server.c -o server

clean:
	rm client server