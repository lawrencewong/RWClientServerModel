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

int sockfd,n;
struct sockaddr_in servaddr,cliaddr;


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
   
   // Setting up the reader and writer thread arrays. 
   thread_data writers_thread_data[num_writers];
   thread_data readers_thread_data[num_readers];
   pthread_t writers_thread[num_writers];
   pthread_t readers_thread[num_readers];

   // Setting up the reader threads.
   for(i = 0; i <num_readers; i++){
      readers_thread_data[i].thread_id = i;
      readers_thread_data[i].iterations = num_iterations;
      readers_thread_data[i].writers = num_writers;
      readers_thread_data[i].readers = num_readers;
      readers_thread_data[i].filename = malloc(sizeof(filename));
      readers_thread_data[i].filename = filename;
      readers_thread_data[i].dest = malloc(sizeof(argv[1]));
      readers_thread_data[i].dest = argv[1];
      ret = pthread_create(&readers_thread[i], 0, readNumber, &readers_thread_data[i]);
      if(ret != 0){
         printf("Create pthread error!\n");
         exit(1);
      }
   }

   // Setting up the writer threads.
   for(i = 0; i <num_writers; i++){
      writers_thread_data[i].thread_id = i;
      writers_thread_data[i].iterations = num_iterations;
      writers_thread_data[i].writers = num_writers;
      writers_thread_data[i].readers = num_readers;
      writers_thread_data[i].filename = malloc(sizeof(filename));
      writers_thread_data[i].filename = filename;
      writers_thread_data[i].dest = malloc(sizeof(argv[1]));
      writers_thread_data[i].dest = argv[1];
      ret = pthread_create(&writers_thread[i], 0, increment, &writers_thread_data[i]);
      if(ret != 0){
         printf("Create pthread error!\n");
         exit(1);
      }
   }




   // for(i=0;i<num_iterations;i++){

   //    sockfd=socket(AF_INET,SOCK_DGRAM,0);

   //    bzero(&servaddr,sizeof(servaddr));
   //    servaddr.sin_family = AF_INET;
   //    servaddr.sin_addr.s_addr=inet_addr(argv[1]);
   //    servaddr.sin_port=htons(32000);



   //    sendto(sockfd,sendline, 10000,0,
   //        (struct sockaddr *)&servaddr,sizeof(servaddr));
   //    n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);
   //    recvline[n]=0;
   //    fputs(recvline,stdout);
   // } 


   // Cleaning up the simulation.
   for(i=0;i<num_writers;i++){
      pthread_join(writers_thread[i],NULL);
   }
   for(i=0;i<num_readers;i++){
      pthread_join(readers_thread[i],NULL);
   }

   pthread_exit(NULL);
   return 0;
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

// The writer thread function that reads the value of the binary file pertaining to that writer and increments that value.
void* increment(void* parameter){
   thread_data * cur_thread;
   FILE *fp;
   cur_thread = (thread_data *)parameter;
   int value = 0;
   int i;
   int k;

   char sendline[1000];
   char recvline[1000];
   char buffer[1000];

   sprintf(sendline, "%d", 2); 
   strcat(sendline,"|");
   pid_t pid = getpid();
   sprintf(buffer, "%d", pid);
   strcat(sendline,buffer);
   strcat(sendline,"|");
   strcat(sendline,"w");
   strcat(sendline,"|");
   strcat(sendline,filename);

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(cur_thread->dest);
   servaddr.sin_port=htons(32000);



   sendto(sockfd,sendline, 10000,0,
    (struct sockaddr *)&servaddr,sizeof(servaddr));
   n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);
   recvline[n]=0;
   fputs(recvline,stdout);

   for(k=1;k<=cur_thread->iterations;k++){

      fp = fopen(cur_thread->filename,"rb+");

      for( i = 0; i < cur_thread->writers; i++){
         fseek(fp,sizeof(int)*i,SEEK_SET);
         fread(&value, sizeof(int), 1, fp);

         if( i == cur_thread->thread_id){
            value++;
            fseek(fp,sizeof(int)*i,SEEK_SET);
            fwrite(&value, sizeof(int), 1, fp);
         }
      }
      fclose(fp);
      sleep(rand()%5);
   }
}

// The reader thread function that reads all the values of each writer's value and prints it to the screen.
void* readNumber(void* parameter){
   thread_data * cur_thread;
   cur_thread = (thread_data *)parameter;
   int contents[cur_thread->writers];
   FILE *fp;
   char *contents_string = malloc(sizeof(int)*cur_thread->writers);
   char temp[sizeof(int)];
   int i;
   int k;
   for(k=1;k<=cur_thread->iterations;k++){

      strcpy(contents_string,"");
      fp = fopen(cur_thread->filename,"rb+");
      fread(contents, sizeof(int),cur_thread->writers, fp);
      for(i=0;i<cur_thread->writers;i++){
         sprintf(temp, "%d ",contents[i]);
         strcat(contents_string,temp);

      }
      fclose(fp);
      printf("Iteration #: %d Reader Thread ID: %d contents: %s \n",k,cur_thread->thread_id+1, contents_string);
      fflush(stdout);
      sleep(rand()%5);

   }
   free(contents_string);
}