all:	a2server a2client

a2server:	a2server.c
	gcc -g -pthread -std=c99 -o a2server a2server.c

a2client:	a2client.c
	gcc -g -pthread -std=c99 -lm -o a2client a2client.c

clean:
	rm a2server
	rm a2client
	rm *.bin