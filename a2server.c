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

#define MAX_CLIENTS 10

ticketNode clientQueues[10];

int main(int argc, char**argv)
{
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   socklen_t len;
   char mesg[1000];
   char *token;
   const char delim[2] = "|";
   char * filename;
   char requestType;
   int clientID;
   int pid;
   int clientGroups[MAX_CLIENTS] = {0};
   int i;
   

   
   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
   servaddr.sin_port=htons(32000);
   bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));


   while(1)
   {
      printf("Waiting\n");
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
      requestType = token[0];
      token = strtok(NULL, delim);
      filename = malloc(sizeof(token));
      filename = token;

      printf("CID: %d PID: %d RT: %c FILE: %s\n", clientID, pid, requestType, filename);
      
      for(i=0;i<MAX_CLIENTS;i++){
         if(clientGroups[i] == 0){
            clientGroups[i] = pid;
            printf("New Client\n");
            startClientQueue(pid, requestType, i);
            break;
         }else if(clientGroups[i] == pid){
            printf("Already have seen this client\n");
            break;
         }
      }
      printf("-------------------------------------------------------\n");
   }
}

void startClientQueue(int pid, char requestType, int index){
   clientQueues[index] = malloc(sizeof(ticketNode));
   clientQueues[index].pid = pid;
   clientQueues[index].requestType = requestType;
   clientQueues[index].head = malloc(sizeof(ticketNode));
   clientQueues[index].head = NULL;
   clientQueues[index].ahead = malloc(sizeof(ticketNode));
   clientQueues[index].ahead = NULL;
}