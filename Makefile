all: client server

client: client.o
	gcc -o client client.o -g

server: server.o payload.o
	gcc -o server server.o payload.o -g

payload.o: payload.c payload.h
	gcc payload.c -c -g

client.o: client.c client.h
	gcc client.c -c -g

server.o: server.c server.h
	gcc server.c -c -g

clean:
	rm -f *.o client server