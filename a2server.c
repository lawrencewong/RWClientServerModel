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

ticketNode * clientQueues[MAX_CLIENTS];
clientGroupInfo clientGroups[MAX_CLIENTS];
struct sockaddr_in servaddr,cliaddr;

int main(int argc, char**argv)
{
   int sockfd,n;
   
   socklen_t len;
   char mesg[1000];
   char *token;
   const char delim[2] = "|";
   char * filename;
   char requestType;
   int clientID;
   int pid;
   int i;
   


   for (i = 0; i < MAX_CLIENTS; i++)
   {
      clientGroups[i].pid = 0;
      clientGroups[i].numActiveReaders = 0;
      clientGroups[i].activeWriter = 0;

      clientQueues[i] = malloc(sizeof(ticketNode));
      clientQueues[i]->pid = 0;
      clientQueues[i]->requestType = '\0';
      clientQueues[i]->head = NULL;
      clientQueues[i]->next = NULL;
   }
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
      
      printf("-------------------------------------------------------\n");
      mesg[n] = 0;
      
      printf("Received the following:\n");

      printf("CLient ID: %s\n",mesg);
      token = strtok(mesg, delim);
      pid = atoi(mesg);
      token = strtok(NULL, delim);
      requestType = token[0];
      token = strtok(NULL, delim);
      filename = malloc(sizeof(token));
      filename = token;

      printf("PID: %d RT: %c FILE: %s\n", pid, requestType, filename);
      
      // REQUEST
      if(requestType == 'r' || requestType == 'w'){
         for(i=0;i<MAX_CLIENTS;i++){

            if(clientGroups[i].pid == pid){
               printf("Already have seen this client\n");
               addToClientQueue(pid, requestType, i);
               break;
            }else if(clientGroups[i].pid == 0){
               clientGroups[i].pid = pid;
               printf("New Client\n");
               startClientQueue(pid, requestType, i);
               break;
            }
         }
         
         runProcess(sockfd);

         // for(i=0;i<MAX_CLIENTS;i++){
         //       printf("QUEUE CHECK PID: %d\n", clientQueues[i]->pid);
         // }
      }else if(requestType == 'x'){ /// RELEASE
         for(i=0;i<MAX_CLIENTS;i++){
            if(clientGroups[i].pid == pid){
               if(clientGroups[i].activeWriter == 0){ // ONLY RELEASE WHEN WRITERS ARE DONE
                  releaseClientQueue(i);
               }
            }
            break;
         }
      }

      printf("-------------------------------------------------------\n");
   }
}

void startClientQueue(int pid, char requestType, int index){
   clientQueues[index]->pid = pid;
   clientQueues[index]->requestType = requestType;
   clientQueues[index]->head = malloc(sizeof(ticketNode));
   clientQueues[index]->head = NULL;
   clientQueues[index]->next = malloc(sizeof(ticketNode));
   clientQueues[index]->next = NULL;
}

void addToClientQueue(int pid, char requestType, int index){
   if(clientQueues[index]->pid == 0){
      clientQueues[index]->pid = pid;
      clientQueues[index]->requestType = requestType;
      clientQueues[index]->next = NULL;
   }else{
      ticketNode * temp;
      ticketNode * current;
      temp = malloc(sizeof(ticketNode));
      temp->pid = pid;
      temp->requestType = requestType;
      current = malloc(sizeof(ticketNode));
      current = clientQueues[index];
      while(current->next != NULL){
         current = current->next;
      }
      current->next = temp;
      temp->next = NULL;
   }
}

// MAKE POP OFF FUNCTION
void runProcess(int sockfd){
   sendto(sockfd,"AWK",3,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));

}

void releaseClientQueue(int index){
   // Writer
   // if(clientGroups[index].numActiveReaders == 0){

   //       // DECREMENT WRITER
   //    // DO WHATEVER
   // }else{ // Reader - Check for other readers
   //       // DECREMENT READER 
   //    // DO ALL UNTIL WRITER
   // }

   // ticketNode * current;
   // current = malloc(sizeof(ticketNode));
   // current = clientQueues[index];

   // while(current->requestType == 'r'){
   //    sendto(sockfd,mesg,n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
   //    current = current->next;
   // }
}