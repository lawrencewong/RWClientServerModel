/* Sample UDP client */

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
   int num_iterations = 0;
   char filename[256];
   int num_processes = 0;
   int num_writers = 0;
   int num_readers = 0;
   int i = 0;
   int ret = 0;
   FILE *fp;


   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   char sendline[1000];
   char recvline[1000];


   

   

   if (argc != 2)
   {
      printf("usage:  udpcli <IP address>\n");
      exit(1);
   }

   printf("How many iterations?\n");
   scanf("%d",&num_iterations);
   printf("How many processes?\n");
   scanf("%d",&num_processes);
   printf("What is the file name?\n");
   scanf("%s",filename);

   num_writers = 0.3 * num_processes;
   num_readers = num_processes - num_writers;
   printf("Number of iterations: %d\n",num_iterations);
   printf("Number of processes: %d Number of Writers: %d Number of Readers: %d \n",num_processes, num_writers, num_readers);
   printf("Filename: %s\n",filename);

   initializeFile(num_writers, filename);
   
   packet sendpacket;
   sendpacket.clientID = 2;
   sendpacket.requestType = 'w';
   sendpacket.filename = malloc(sizeof(filename));

   
   for(i=0;i<num_iterations;i++){
      sockfd=socket(AF_INET,SOCK_DGRAM,0);

      bzero(&servaddr,sizeof(servaddr));
      servaddr.sin_family = AF_INET;
      servaddr.sin_addr.s_addr=inet_addr(argv[1]);
      servaddr.sin_port=htons(32000);

      sendpacket.clientID = sockfd;

      sendto(sockfd,&sendpacket,strlen(sendline),0,
          (struct sockaddr *)&servaddr,sizeof(servaddr));
      n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);
      recvline[n]=0;
      fputs(recvline,stdout);
   } 
}

// Filling the shared memory file with 0's.
void* initializeFile(int num_writers, char * filename){
   int fill = 0;
   int i = 0;
   FILE *fp;
   fp =fopen(filename,"w+");

   if(fp != NULL){
      for(i = 0; i < num_writers; i++){
         fwrite(&fill, sizeof(int), 1, fp);
      }
      fclose(fp);
   }else{
      printf("Could not open file.\n");
      exit(1);
   }
}
