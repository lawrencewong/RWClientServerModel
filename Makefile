all:	a2server a2client

a2server:	a2server.c
	gcc -pthread -o a2server a2server.c

a2client:	a2client.c
	gcc -pthread -o a2client a2client.c

clean:
	rm a2server
	rm a2client
	rm *.bin