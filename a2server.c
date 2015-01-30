#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 

#define MAX_CLIENTS 10

int main(void){

	int listenFD = 0, connFD = 0;
	struct sockaddr_in serv_addr;

	char sendBuff[1025];
	time_t ticks;

	listenFD = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(sendBuff, '0', sizeof(sendBuff));
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(5000);

	bind(listenFD, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	listen(listenFD, MAX_CLIENTS);

	while(1){
		
		connFD = accept(listenFD, (struct sockaddr*)NULL, NULL);

		ticks = time(NULL);
		snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
		write(connFD, sendBuff, strlen(sendBuff));

		close(connFD);
		sleep(1);

	}
	return 0;
}
