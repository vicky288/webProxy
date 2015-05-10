/* A simple, TCP based echo client */
#include <netinet/in.h>
#include <stdio.h>  
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

//char proxyServer[] = "10.211.55.25";
char proxyServer[] = "10.176.67.64";

void str_cli(int, char**);

int main(int argc, char **argv) {
  int                sockfd;
  struct sockaddr_in servaddr;
 
  /* creating a socket */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n%s: Error in socket", argv[0]);
    exit(0);
  }

  /* configuring server address structure */
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(9876);

  if (inet_pton(AF_INET, proxyServer, &servaddr.sin_addr) <= 0) {
    printf("\n%s: Error in inet_pton", argv[0]); 
    exit(0);
  }

  /* connecting to the server */
  if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
    printf("\n%s: Error in connect", argv[0]);
    exit(0);
  }

  str_cli(sockfd, argv);
}

void str_cli (int sockfd, char **argv) {
  char sendline[1024], recvline[4096];
  int n;
	
  printf("Enter the url [domainName:port:page] ->");
  /* read a line from stdin */
  if (fgets(sendline, 1024, stdin) == NULL) {
    printf("\n%s: Error in fgets", argv[0]);
    exit(0);
  }

  /* send it to echo server */
  if (write(sockfd, (const void*) sendline, strlen(sendline)) < 0) {
    printf("\n%s: Error in write", argv[0]);
    exit(0);
  }

  /* get the echo from the echo server and display it on the terminal*/
  /*while ( (n = read(sockfd, recvline, 1024)) > 0) {
    recvline[n] = 0; //null terminate the string 
    printf("############## %d %%%%%%%%%%%%\n", n);
    fputs(recvline, stdout);
  }*/
    n = 1;
    while ( n > 0) {
    n = recv(sockfd, recvline, 4096-1, 0);
    recvline[n] = 0; // null terminate the string 
    //printf("############## %d %%%%%%%%%%%%\n", n);
    
    fputs(recvline, stdout);
  }
  
  printf("out..............%d\n",n);
  close(sockfd);
}
