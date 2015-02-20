/* Sample UDP server */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <pthread.h>
#include "a2.h"

int main(int argc, char**argv)
{
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   socklen_t len;
   char mesg[1000];
   char *token;
   const char delim[2] = "|";
   char filename[256];
   char requestType;
   int clientID;
   int pid;
   packet * recvpacketptr;
   sockfd=socket(AF_INET,SOCK_DGRAM,0);
   recvpacketptr = malloc(sizeof(packet));
   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
   servaddr.sin_port=htons(32000);
   bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

   while(1)
   {
      len = sizeof(cliaddr);
      n = recvfrom(sockfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&len);
      sendto(sockfd,"BACK",n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
      printf("-------------------------------------------------------\n");
      mesg[n] = 0;
      
      printf("Received the following:\n");

      printf("CLient ID: %s\n",mesg);
      token = strtok(mesg, delim);
      clientID = atoi(token);
      token = strtok(NULL, delim);
      pid = atoi(token);
      token = strtok(NULL, delim);
      requestType = token;
      filename = token;
      printf("CID: %d PID: %d RT: %c FILE: %s\n", clientID, pid, requestType, filename);
      
      printf("-------------------------------------------------------\n");
   }
}