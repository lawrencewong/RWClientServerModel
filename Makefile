all:	a2server a2client

a2server:	a2server.c
	gcc -o a2server a2server.c

a2client:	a2client.c
	gcc -o a2client a2client.c
