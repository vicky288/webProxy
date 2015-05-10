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
#include <sys/poll.h>


struct web_url {
    char domain_name[30];
    char port_no[10];
    char page[50];
    
};

static void * process_request_client(void *arg);
void getClientInput(int sockfd, struct web_url *new_url);
int split( char * str, char delim, char ***array, int *length );
int connect_web(char * ip_addr, char * port, char * page);
void process_webPage(int soc_client, int soc_web, char * page, char * host);
//##############


int main (int argc, char **argv) {
   /* variables section */
   pthread_t tid;
   int                listenfd, connfd;
   pid_t              childpid;
   socklen_t          clilen;
   struct sockaddr_in cliaddr, servaddr;

   /* creating a socket */
   if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
     printf("\n%s: Error in socket", argv[0]);
     exit(0);
   }

   /* configuring server address structure */
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family      = AF_INET;
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
   servaddr.sin_port        = htons(9876); 

   /* binding our socket to the service port */
   if (bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
     printf("\n%s: Error in bind", argv[0]); 
     exit(0);
   } 

   /* convert our socket to a listening socket */
   if (listen(listenfd, 5) < 0) {
     printf("\n%s: Error in listen", argv[0]); 
     exit(0);
   } 

   for ( ; ; ) {
     clilen = sizeof(cliaddr);

     /* accept a new connection and return a new socket descriptor to 
     handle this new client */
     if ((connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen)) < 0) {
       printf("\n%s: Error in accept", argv[0]); 
       exit(0);
     } 

    pthread_create(&tid, NULL, &process_request_client, (void *) connfd);   
  }
  return 0;
}
  

static void * process_request_client(void *soc_client) {
    struct web_url url_new;
    
    int soc_web;
    pthread_detach(pthread_self());
    
    getClientInput((int) soc_client, &url_new);
    
    printf("1111 Before connect_web \n ip_addr in process_request_client -> %s \n port in process_request_client-> %s \n page in process_request_client->%s\n",
	   url_new.domain_name, url_new.port_no, url_new.page);

    //create socket connection to web server
    soc_web = connect_web(url_new.domain_name, url_new.port_no, url_new.page);
    

    if (soc_web > 0) {
      printf("##########connected to web : %s ###########\n",url_new.domain_name); 
    } else {
      return;
    }

    printf("2222 Before process_webPage \n ip_addr in process_request_client -> %s \n port in process_request_client-> %s \n page in process_request_client->%s\n",
	   url_new.domain_name, url_new.port_no, url_new.page);
    
    //process webPage
    process_webPage((int)soc_client, soc_web, url_new.page, url_new.domain_name);

    
    close(soc_web);
    close((int) soc_client);
    return (NULL);
}
void getClientInput(int sockfd, struct web_url *new_url) {  
    ssize_t n;
    char line[512];
    char **res;
    char* runner;
    int rc, count = 0, x=0;
    /* read from socket */
    n = read(sockfd, (void *) line, 512);

    if (n < 0) {
      printf("\nError in read"); 
      exit(0);
    } else if (n == 0) {
      return;
    } 
    line[n-1] = NULL;
    printf("Yo baby got it:%d\n",n);
    
    //split it and store it variables
    rc = split( line, ':', &res, &count );
    if( rc ) {
    printf("Error: error in spliting \n");
    }

    runner = res[0];
    while (*runner != NULL) {
      new_url->domain_name[x] =  *runner;
      runner ++;
      x++;
    }
    new_url->domain_name[x] = NULL;
    
    x=0;
    runner = res[1];
    while (*runner != NULL) {
      new_url->port_no[x] =  *runner;
      runner ++;
      x++;
    }
    new_url->port_no[x] = NULL;

    x=0;
    runner = res[2];
    while (*runner != NULL) {
      new_url->page[x] =  *runner;
      runner ++;
      x++;
    }
    new_url->page[x] = NULL;

    //close(sockfd);					//closed here earlier
}

int split( char * str, char delim, char ***array, int *length ) {
  char *p;
  char **res;
  int count=0;
  int k=0;
  int index = 0;

  do{
    //printf("%c\n", str[]);
    index++;
  } while (str[index] != NULL);
  str[index] = ':';
  index++;
  str[index] = NULL;

  printf("%s--------------------------\n",str);

  p = str;
  // Count occurance of delim in string
  while( (p=strchr(p,delim)) != NULL ) {
    *p = 0; // Null terminate the deliminator.
    p++; // Skip past our new null
    count++;
  }

  // allocate dynamic array
  res = calloc( 1, count * sizeof(char *));
  if( !res ) return -1;

  p = str;
  for( k=0; k<count; k++ ){
    if( *p ) res[k] = p;  // Copy start of string
    p = strchr(p, 0 );    // Look for next null
    p++; // Start of next string
  }

  *array = res;
  *length = count;

  return 0;
}

int connect_web(char * ip_addr, char * port, char * page_old) {
 
  struct addrinfo hints, *res;
  int sockfd, retVal;
  char * ip_addr_old="www.utdallas.edu";
  char * port_old ="80";
  
  printf("ip_addr in connect_web -> %s \n port in connect_web-> %s \n",ip_addr,port);
  //printf("size= %d\n",strlen(ip_addr));

  // first, load up address structs with getaddrinfo():
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  retVal=getaddrinfo(ip_addr, port, &hints, &res);  
  printf("getaddrinfo Return Val is -> %d\n",retVal);

  // create a socket:
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  
  /* connecting to the server */
  if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
    printf("\n: Error in connect to web page");
    //exit(0);
  }
  return sockfd;
}
void process_webPage(int soc_client, int soc_web, char * page, char * host){
  char arr[200];
  char * sendline; 
  char recvline[4096];
  int n,rv;
  struct pollfd ufds[2];
  
  

  ufds[1].fd = soc_web;
  ufds[1].events = POLLIN; // check for just normal data

  // Prepare the HTTP request
  sendline = arr;
  strcpy(arr,"GET ");
  strcat(arr,page);
  strcat(arr," HTTP/1.1\r\nHost:");
  strcat(arr,host);
  strcat(arr,"\r\n\r\n");

    
  // send it to server 
  printf ("$$$$$$$$$$$HTTP request to be sent$$$$$$$$$$\n%s\n",sendline);
  if (write(soc_web, (const void*) sendline, strlen(sendline)) < 0) {
    printf("\n: Error in write to web server socket");
    exit(0);
  }

  
  // Receive data from Socket
  rv = poll(ufds, 2, 20000);
  while ( rv > 0) {
    if (rv == -1) {
      perror("poll"); // error occurred in poll()
    } else if (rv == 0) {
      printf("Timeout occurred!  No data after 3.5 seconds.\n");
    } else {
      if (ufds[1].revents & POLLIN) {
        recv(soc_web, recvline, 4096-1, 0);
      }
    } 
    fputs(recvline, stdout);
    if (write(soc_client, (const void*) recvline, strlen(recvline)) < 0) {
    printf("\nError in write back to Client");
    exit(0);
  } 
    //printf("mmmmmmmmmmmmmmmmmmmm %d", rv);
    rv = poll(ufds, 2, 20000);
  }
  

  printf("@@@@@@@@@@@@ Out Side @@@@@@@@@@@\n");

  //close(soc_web);     	//closed earlier
}