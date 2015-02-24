/*
a2server.c
Server for Readers and Writers problem using a ticket server
Lawrence Wong
*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "a2.h"

// Max number of clients for now set to 10
#define MAX_CLIENTS 10

// Setting up global queues and queue infos as well as addresses
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
   int pid;
   int i;
   int thread_id;
   int iteration;
   struct sockaddr_in cliaddr;
   
   // Seting up array of queues and info
   for (i = 0; i < MAX_CLIENTS; i++)
   {
      clientGroups[i].pid = 0;
      clientGroups[i].numActiveReaders = 0;
      clientGroups[i].activeWriter = 0;
      clientQueues[i] = malloc(sizeof(ticketNode));
      clientQueues[i]->pid = 0;
      clientQueues[i]->requestType = '\0';
      clientQueues[i]->next = NULL;
      clientQueues[i]->socketFD = 0;
      clientQueues[i]->thread_id = 0;
      clientQueues[i]->iteration = 0;
   }

   // Creating server socket
   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   // Setting up server address
   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
   servaddr.sin_port=htons(32000);
   bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

   // Main server loop
   while(1){
      
      // Refreshing client address
      memset((char*)&cliaddr, 0, sizeof(cliaddr));

      printf("Listening for requests or releases.\n");

      // Wait for release or request
      len = sizeof(cliaddr);
      n = recvfrom(sockfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&len);
      
      printf("-------------------------------------------------------\n");
      mesg[n] = 0;
      
      // Recieves the info on a thread and its release status
      printf("Received the following: %s\n", mesg);
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

      printf("PID: %d RequestType: %c Filename: %s Thread: %d Iteration: %d Release Flag: %c.\n", pid, requestType, filename, thread_id, iteration, release);
      
      // If a request is sent in
      if((requestType == 'r' || requestType == 'w')&& release == 'X'){
         for(i=0;i<MAX_CLIENTS;i++){
            if(clientGroups[i].pid == pid){
               printf("Already have seen this client.\n");
               addToClientQueue(pid, requestType, i, sockfd, thread_id, iteration, cliaddr);
               runProcess(i);
               break;
            }else if(clientGroups[i].pid == 0){
               clientGroups[i].pid = pid;
               printf("New Client.\n");
               startClientQueue(pid, requestType, i, sockfd, thread_id, iteration, cliaddr);
               runProcess(i);
               break;
            }
         }
      }else if(release == 'O'){ // If release is sent in
         for(i=0;i<MAX_CLIENTS;i++){
            if(clientGroups[i].pid == pid){
               releaseClientQueue(i,pid, requestType, release);
               break;
            }
         }
      }
      printf("-------------------------------------------------------\n");
   }
}

// Starts a client queue
void startClientQueue(int pid, char requestType, int index, int socketFD, int thread_id, int iteration, struct sockaddr_in cliaddr){
   clientQueues[index]->pid = pid;
   clientQueues[index]->requestType = requestType;
   clientQueues[index]->next = malloc(sizeof(ticketNode));
   clientQueues[index]->next = NULL;
   clientQueues[index]->socketFD= socketFD;
   clientQueues[index]->thread_id= thread_id;
   clientQueues[index]->iteration= iteration;
   clientQueues[index]->cliaddr = cliaddr;
}

// Adds to the queue
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
      printf("Adding request to queue.\n");
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
         printf("QUEUE: PID: %d RT: %c THREAD: %d ITERATION: %d\n",current->pid, current->requestType, current->thread_id, current->iteration);
         current = current->next;
      }
      printf("QUEUE: PID: %d RT: %c THREAD: %d ITERATION: %d\n",current->pid, current->requestType, current->thread_id, current->iteration);
}

// Run the head of a given queue. Dequeues if a thread is allowed to run
void runProcess(int index){

   printf("Next ticket to run. PID: %d RT: %c THREAD: %d ITERATION: %d WF: %d NR: %d\n", clientQueues[index]->pid, clientQueues[index]->requestType, clientQueues[index]->thread_id, clientQueues[index]->iteration, clientGroups[index].activeWriter, clientGroups[index].numActiveReaders);
   
   // Runs a writer. If it has sole possesion.
   if(clientQueues[index]->requestType == 'w' && clientGroups[index].numActiveReaders == 0 && clientGroups[index].activeWriter == 0){
      clientGroups[index].activeWriter = 1;
      sendto(clientQueues[index]->socketFD,"AWK",3,0,(struct sockaddr *)&clientQueues[index]->cliaddr,sizeof(clientQueues[index]->cliaddr));
      printf("SENDING AWK TO WRITER: %d ITERATION: %d\n", clientQueues[index]->thread_id, clientQueues[index]->iteration);
      if(clientQueues[index]->next == NULL){
         clientQueues[index] = NULL;
      }else{
         clientQueues[index] = clientQueues[index]->next;
      }
   }else if(clientQueues[index]->requestType == 'r' && clientGroups[index].activeWriter == 0){ // Runs a reader, if no writers are writing.
      clientGroups[index].numActiveReaders++;
      sendto(clientQueues[index]->socketFD,"AWK",3,0,(struct sockaddr *)&clientQueues[index]->cliaddr,sizeof(clientQueues[index]->cliaddr));
      printf("SENDING AWK TO READER: %d ITERATION: %d\n", clientQueues[index]->thread_id, clientQueues[index]->iteration);
      if(clientQueues[index]->next == NULL){
         clientQueues[index] = NULL;
      }else{
         clientQueues[index] = clientQueues[index]->next;
      }
   }
}

// Accepts the release from the client looks to next in queue to see if they are suitable to be dequeued
void releaseClientQueue(int index, int pid, char requestType, char release){

   printf("Released Received from PID: %d RT: %c RELEASE: %c WF: %d NR: %d\n", pid, requestType, release, clientGroups[index].activeWriter, clientGroups[index].numActiveReaders);
   if(requestType == 'w' && release == 'O'){
      // Decrement writer flag
      clientGroups[index].activeWriter = 0;
   }else if(requestType == 'r' && release == 'O'){
      // Decrement reader flag 
      clientGroups[index].numActiveReaders--;
   }

   if(clientQueues[index] != NULL){
      ticketNode * temp;
      temp = malloc(sizeof(ticketNode));
      temp = clientQueues[index];

      // Writer next
      if(temp->requestType == 'w' && clientGroups[index].activeWriter == 0 && clientGroups[index].numActiveReaders == 0){
         runProcess(index);
      }else if(temp->requestType == 'r'  && clientGroups[index].activeWriter == 0){// Reader next
         
         while(temp != NULL){
            if(temp->requestType == 'r'){
               runProcess(index);               
            }
            temp = temp->next;
         }
      }
   }
}