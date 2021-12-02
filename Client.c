
/*****************************************************************************/
/*** tcpclient.c                                                           ***/
/***                                                                       ***/
/*** Demonstrate an TCP client.                                            ***/
/*****************************************************************************/

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include<ctype.h>
#include<unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>


void panic(char *msg);
#define panic(m)	{perror(m); abort();}
#define LENGTH 2048



//Variables
char name[32];
volatile sig_atomic_t flag = 0;

void str_trim_lf (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}
//----------------------------------------------------------------------
void str_overwrite_stdout() {
  printf("%s", "> ");
  fflush(stdout);
}

//----------------------------------------------------------------------
void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}
//---------------------------------------------------------------
void send_msg_handler(void *arg){

	char message[LENGTH] = {};
	char buffer[LENGTH + 32] = {};
	int sd =*(int*)arg; 
	
while(1) {
	str_overwrite_stdout();
	fgets(message, LENGTH, stdin);
	str_trim_lf(message, LENGTH);
	
    if (strcmp(message, "exit") == 0) {
			break;
    } 
    else {
      sprintf(buffer, "%s: %s \n", name, message);
      send(sd, buffer, strlen(buffer), 0);
    }

	bzero(message, LENGTH);
    	bzero(buffer, LENGTH + 32);
  }
  catch_ctrl_c_and_exit(2);
}
//----------------------------------------------------------------------------------
void recv_msg_handler(void *arg) {
	char message[LENGTH] = {};
	int sd =*((int*)arg); 

  while (1) {
	int receive = recv(sd, message, LENGTH, 0);
    if (receive > 0) {
      printf("%s", message);
      str_overwrite_stdout();
    }
    else if (receive == 0) {
			break;
    } 
    else {
			// -1
	}
	memset(message, 0, sizeof(message));
  }
}

/****************************************************************************/
/*** This program opens a connection to a server using either a port or a ***/
/*** service.  Once open, it sends the message from the command line.     ***/
/*** some protocols (like HTTP) require a couple newlines at the end of   ***/
/*** the message.                                                         ***/
/*** Compile and try 'tcpclient lwn.net http "GET / HTTP/1.0" '.          ***/
/****************************************************************************/
int main(int count, char *args[])
{	struct hostent* host;
	struct sockaddr_in addr;
	int sd=0 , port;

	if ( count != 3 )
	{
		printf("usage: %s <servername> <protocol or portnum>\n", args[0]);
		exit(0);
	}



	/*---Get server's IP and standard service connection--*/
	host = gethostbyname(args[1]);
	//printf("Server %s has IP address = %s\n", args[1],inet_ntoa(*(long*)host->h_addr_list[0]));
	
	if ( !isdigit(args[2][0]) ) // isdigit verify if the argument is a digit or  not and return a zero if is not 
	{
		struct servent *srv = getservbyname(args[2], "tcp");//getserver return a pointer to  a struct requested
		if ( srv == NULL )
			panic(args[2]);
		printf("%s: port=%d\n", srv->s_name, ntohs(srv->s_port));
		port = srv->s_port;
	}
	else
		port = htons(atoi(args[2]));
	
	signal(SIGINT, catch_ctrl_c_and_exit);

		printf("Please enter your name: ");
		fgets(name, 32, stdin);
		str_trim_lf(name, strlen(name));
		
		
		if (strlen(name) > 32 || strlen(name) < 2){
		printf("Name must be less than 30 and more than 2 characters.\n");
		return EXIT_FAILURE;
	}

	/*---Create socket and connect to server---*/
	sd = socket(AF_INET, SOCK_STREAM, 0);        /* create socket */
		
	if ( sd < 0 )
		panic("socket");


	char *ip = "127.0.0.1";
	addr.sin_family = AF_INET;        /* select internet protocol */
	addr.sin_port = port;                       /* set the port # */
	addr.sin_addr.s_addr = inet_addr(ip); /* set the addr */

	/*---If connection successful, send the message and read results---*/
	if ( connect(sd, (struct sockaddr*)&addr, sizeof(addr)) == 0)
	{	
		
        send(sd, name, 32, 0);
		printf("welcome aboard\n");
		
		pthread_t thread_send_message;
		pthread_t thread_recv_message;
		
	if(pthread_create(&thread_send_message, NULL, (void *)send_msg_handler, (void *) &sd) != 0){
		printf("Erro: pthread\n");
		return 0;
	}
	
	if(pthread_create(&thread_recv_message, NULL, (void *)recv_msg_handler, (void *) &sd) != 0){
		printf("Erro: pthread\n");
		return 0;
	}
	
	while (1){
		if(flag){
			printf("\nBye\n");
			break;
			}
			}
		close(sd);
		
		return 1;
}

}
