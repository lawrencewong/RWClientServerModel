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
#include "a2.h"

int main(int argc, char**argv)
{
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   char sendline[1000];
   char recvline[1000];
   pid_t childPID;
   packet sendpacket;
   sendpacket.clientID = 2;
   sendpacket.requestType = 'w';
   sendpacket.filename = malloc(sizeof("test.txt"));

   if (argc != 2)
   {
      printf("usage:  udpcli <IP address>\n");
      exit(1);
   }

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(argv[1]);
   servaddr.sin_port=htons(32000);

   // while (fgets(sendline, 10000,stdin) != NULL)
   // {
   childPID = fork();
   if(childPID >= 0){

      if(childPID == 0){ //CHILD
         sendto(sockfd,&sendpacket,strlen(sendline),0,
             (struct sockaddr *)&servaddr,sizeof(servaddr));
         n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);
         recvline[n]=0;
         fputs(recvline,stdout);
      }
      else{ //PARENT
         printf("PARENT\n");
      }
   }
      
   // }
}