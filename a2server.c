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
   char release;
   int clientID;
   int pid;
   int i;
   int thread_id;
   int iteration;
   struct sockaddr_in cliaddr;
   


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
      memset((char*)&cliaddr, 0, sizeof(cliaddr));
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
      token = strtok(NULL, delim);
      release = token[0];

      printf("PID: %d RT: %c FILE: %s THREAD: %d ITERATION: %d RELEASE %c\n", pid, requestType, filename, thread_id, iteration, release);
      
      // REQUEST
      if((requestType == 'r' || requestType == 'w')&& release == 'X'){
         for(i=0;i<MAX_CLIENTS;i++){

            if(clientGroups[i].pid == pid){
               printf("Already have seen this client\n");
               addToClientQueue(pid, requestType, i, sockfd, thread_id, iteration, cliaddr);
               temp = clientQueues[i];
               //    while(temp->thread_id != thread_id && temp->iteration != iteration && temp->requestType != requestType){
               //       temp = temp->next;
               // }
               runProcess(i, temp);
               break;
            }else if(clientGroups[i].pid == 0){
               clientGroups[i].pid = pid;
               printf("New Client\n");
               startClientQueue(pid, requestType, i, sockfd, thread_id, iteration, cliaddr);
               temp = clientQueues[i];
               // while(temp->thread_id != thread_id && temp->iteration != iteration && temp->requestType != requestType){
               //       temp = temp->next;
               // }
               runProcess(i, temp);
               break;
            }
         }
         
         

         // for(i=0;i<MAX_CLIENTS;i++){
         //       printf("QUEUE CHECK PID: %d\n", clientQueues[i]->pid);
         // }
      }else if(release == 'O'){ /// RELEASE
         for(i=0;i<MAX_CLIENTS;i++){
            if(clientGroups[i].pid == pid){
               releaseClientQueue(i,pid, requestType, release);
            }
            break;
         }
      }

      printf("-------------------------------------------------------\n");
   }
}

void startClientQueue(int pid, char requestType, int index, int socketFD, int thread_id, int iteration, struct sockaddr_in cliaddr){
   clientQueues[index]->pid = pid;
   clientQueues[index]->requestType = requestType;
   clientQueues[index]->head = malloc(sizeof(ticketNode));
   clientQueues[index]->head = NULL;
   clientQueues[index]->next = malloc(sizeof(ticketNode));
   clientQueues[index]->next = NULL;
   clientQueues[index]->socketFD= socketFD;
   clientQueues[index]->thread_id= thread_id;
   clientQueues[index]->iteration= iteration;
   clientQueues[index]->cliaddr = cliaddr;

}

void addToClientQueue(int pid, char requestType, int index, int socketFD, int thread_id, int iteration, struct sockaddr_in cliaddr){
   if(clientQueues[index] == NULL){
      clientQueues[index] = malloc(sizeof(ticketNode));
      clientQueues[index]->pid = pid;
      clientQueues[index]->requestType = requestType;
      clientQueues[index]->next = malloc(sizeof(ticketNode));
      clientQueues[index]->next = NULL;
      clientQueues[index]->socketFD= socketFD;
      clientQueues[index]->thread_id= thread_id;
      clientQueues[index]->iteration= iteration;
      clientQueues[index]->cliaddr = cliaddr;
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
      temp->cliaddr = cliaddr;
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
         printf("QUEUE: PID: %d RT: %c THREAD: %d ITERATION: %d SOCKETFD: %d\n",current->pid, current->requestType, current->thread_id, current->iteration, current->socketFD);
         current = current->next;
      }
      printf("QUEUE: PID: %d RT: %c THREAD: %d ITERATION: %d SOCKETFD:%d\n",current->pid, current->requestType, current->thread_id, current->iteration, current->socketFD);


}


void runProcess(int index, ticketNode * ticketToRun){

   printf("ticketToRun DATA: PID: %d RT: %c THREAD: %d ITERATION: %d WF: %d NR: %d\n", ticketToRun->pid, ticketToRun->requestType, ticketToRun->thread_id, ticketToRun->iteration, clientGroups[index].activeWriter, clientGroups[index].numActiveReaders);
   
   // RUN WRITER
   if(ticketToRun->requestType == 'w' && clientGroups[index].numActiveReaders == 0 && clientGroups[index].activeWriter == 0){
      printf("RUNNING WRITER\n");
      clientGroups[index].activeWriter = 1;
      
      sendto(clientQueues[index]->socketFD,"AWK",3,0,(struct sockaddr *)&clientQueues[index]->cliaddr,sizeof(clientQueues[index]->cliaddr));
      printf("SENDING AWK TO WRITER: %d ITERATION %d SOCKET: %d\n", ticketToRun->thread_id, ticketToRun->iteration, ticketToRun->socketFD);
      printf("SENDING AWk TO WRITER: %d ITERATION %d SOCKET: %d\n", clientQueues[index]->thread_id, clientQueues[index]->iteration, clientQueues[index]->socketFD);
      printf("clientQueues[index] DATA: PID: %d RT: %c THREAD: %d ITERATION: %d\n", clientQueues[index]->pid, clientQueues[index]->requestType, clientQueues[index]->thread_id, clientQueues[index]->iteration);
      if(clientQueues[index]->next == NULL){
         clientQueues[index] = NULL;
      }else{
         clientQueues[index] = clientQueues[index]->next;
         printf("clientQueues[index] DATA: PID: %d RT: %c THREAD: %d ITERATION: %d\n", clientQueues[index]->pid, clientQueues[index]->requestType, clientQueues[index]->thread_id, clientQueues[index]->iteration);

      }
      

   }else if(ticketToRun->requestType == 'r' && clientGroups[index].activeWriter == 0){
      printf("RUNNING READER\n");
      
      clientGroups[index].numActiveReaders++;
      
      sendto(clientQueues[index]->socketFD,"AWK",3,0,(struct sockaddr *)&clientQueues[index]->cliaddr,sizeof(clientQueues[index]->cliaddr));
      printf("SENDING AWK TO READER: %d ITERATION %d SOCKET: %d\n", ticketToRun->thread_id, ticketToRun->iteration, ticketToRun->socketFD);
      printf("SENDING AWK TO READER: %d ITERATION %d SOCKET: %d\n", clientQueues[index]->thread_id, clientQueues[index]->iteration, clientQueues[index]->socketFD);
      printf("clientQueues[index] DATA: PID: %d RT: %c THREAD: %d ITERATION: %d\n", clientQueues[index]->pid, clientQueues[index]->requestType, clientQueues[index]->thread_id, clientQueues[index]->iteration);
      if(clientQueues[index]->next == NULL){
         clientQueues[index] = NULL;
      }else{
         clientQueues[index] = clientQueues[index]->next;
         printf("clientQueues[index] DATA: PID: %d RT: %c THREAD: %d ITERATION: %d\n", clientQueues[index]->pid, clientQueues[index]->requestType, clientQueues[index]->thread_id, clientQueues[index]->iteration);
      }
   }
   // Run Reader
   

}

// MAKE POP OFF FUNCTION
void releaseClientQueue(int index, int pid, char requestType, char release){
   // Writer

   printf("GOT RELEASE FROM: PID: %d RT: %c RELEASE: %c WF: %d NR: %d\n", pid, requestType, release, clientGroups[index].activeWriter, clientGroups[index].numActiveReaders);
   if(requestType == 'w' && release == 'O'){


         // DECREMENT WRITER
      clientGroups[index].activeWriter = 0;
      // DO WHATEVER
   }else if(requestType == 'r' && release == 'O'){
      // DECREMENT READER 
      clientGroups[index].numActiveReaders--;
   }else{ 
      printf("NOT SUPPOSED TO HAPPEN\n");
   }

   if(clientQueues[index] != NULL){
      ticketNode * temp;
      temp = malloc(sizeof(ticketNode));
      temp = clientQueues[index];

      // Writer next
      if(temp->requestType == 'w' && clientGroups[index].activeWriter == 0 && clientGroups[index].numActiveReaders == 0){
         runProcess(index, clientQueues[index]);
      }else if(temp->requestType == 'r'  && clientGroups[index].activeWriter == 0){// Reader next
         
         while(temp != NULL){
            if(temp->requestType == 'r'){
               runProcess(index, temp);               
            }
            temp = temp->next;
         }
      }
   }
   


   // CHECK QUEUE FOR READERS
   // IF Next one is writer set flags and stop search let it run
   // IF READER, let is run until you hit null or rwriter
}