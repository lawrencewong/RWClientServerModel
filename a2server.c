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
   int thread_id;
   int iteration;
   


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
      clientQueues[i]->socketFD = 0;
      clientQueues[i]->thread_id = 0;
      clientQueues[i]->iteration = 0;
   }
   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
   servaddr.sin_port=htons(32000);
   bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

   

   while(1)
   {
      ticketNode * temp;
   temp = malloc(sizeof(ticketNode));
   temp->pid = 0;
   temp->requestType = '\0';
   temp->socketFD = 0;
   temp->head = malloc(sizeof(ticketNode));
   temp->head = NULL;
   temp->next = malloc(sizeof(ticketNode));
   temp->next = NULL;
   temp->thread_id = 0;
   temp->iteration = 0;

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
      token = strtok(NULL, delim);
      thread_id = atoi(token);
      token = strtok(NULL, delim);
      iteration = atoi(token);

      printf("PID: %d RT: %c FILE: %s THREAD: %d ITERATION: %d\n", pid, requestType, filename, thread_id, iteration);
      
      // REQUEST
      if(requestType == 'r' || requestType == 'w'){
         for(i=0;i<MAX_CLIENTS;i++){

            if(clientGroups[i].pid == pid){
               printf("Already have seen this client\n");
               addToClientQueue(pid, requestType, i, sockfd, thread_id, iteration);
               temp = clientQueues[i];
                  while(temp->thread_id != thread_id && temp->iteration != iteration && temp->requestType != requestType){
                     temp = temp->next;
               }
               runProcess(i, temp);
               break;
            }else if(clientGroups[i].pid == 0){
               clientGroups[i].pid = pid;
               printf("New Client\n");
               startClientQueue(pid, requestType, i, sockfd, thread_id, iteration);
               temp = clientQueues[i];
               while(temp->thread_id != thread_id && temp->iteration != iteration && temp->requestType != requestType){
                     temp = temp->next;
               }
               runProcess(i, temp);
               break;
            }
         }
         
         

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

void startClientQueue(int pid, char requestType, int index, int socketFD, int thread_id, int iteration){
   clientQueues[index]->pid = pid;
   clientQueues[index]->requestType = requestType;
   clientQueues[index]->head = malloc(sizeof(ticketNode));
   clientQueues[index]->head = NULL;
   clientQueues[index]->next = malloc(sizeof(ticketNode));
   clientQueues[index]->next = NULL;
   clientQueues[index]->socketFD= socketFD;
   clientQueues[index]->thread_id= thread_id;
   clientQueues[index]->iteration= iteration;

}

void addToClientQueue(int pid, char requestType, int index, int socketFD, int thread_id, int iteration){
   if(clientQueues[index]->pid == 0){
      clientQueues[index]->pid = pid;
      clientQueues[index]->requestType = requestType;
      clientQueues[index]->next = NULL;
      clientQueues[index]->socketFD= socketFD;
      clientQueues[index]->thread_id= thread_id;
      clientQueues[index]->iteration= iteration;
   }else{
      printf("ADDING TO QUEUE\n");
      ticketNode * temp;
      ticketNode * current;
      temp = malloc(sizeof(ticketNode));
      temp->pid = pid;
      temp->requestType = requestType;
      temp->socketFD = socketFD;
      temp->thread_id = thread_id;
      temp->iteration = iteration;
      current = malloc(sizeof(ticketNode));
      current = clientQueues[index];
      while(current->next != NULL){
         current = current->next;
      }
      current->next = temp;
      temp->next = NULL;
   }
    ticketNode * current;
    current = malloc(sizeof(ticketNode));
      current = clientQueues[index];
      while(current->next != NULL){
         printf("QUEUE: PID: %d RT: %c THREAD: %d ITERATION: %d\n",current->pid, current->requestType, current->thread_id, current->iteration);
         current = current->next;
      }


}

// MAKE POP OFF FUNCTION
void runProcess(int index, ticketNode * ticketToRun){

   printf("ticketToRun DATA: PID: %d RT: %c THREAD: %d ITERATION: %d\n", ticketToRun->pid, ticketToRun->requestType, ticketToRun->thread_id, ticketToRun->iteration);
   sendto(ticketToRun->socketFD,"AWK",3,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));

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