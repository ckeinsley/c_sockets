all: client server

client: client.c client.h
	gcc client.c -o client

server: server.c server.h
	gcc server.c -o server

clean:
	rm client server