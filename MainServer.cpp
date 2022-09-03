
#include "Server.h"


int sockfd;   //socket descriptor

Server S;
static char *LOGFILE = "request.log";    //file to store the information about client requests


/*
   FUNCTION NAME	: signalHandler 
 
   DESCRIPTION	: Handles CTRL+c signal and stops the server program.It closes
 		          server socket and request log file before exiting from the 
 		          program
   PARAMETERS	: sig
 
   RETURN 		: void
 
 */

void signalHandler(int sig)
{
	if(sig==SIGINT)
	{
		printf("\n---------SERVER PROGRAM STOPPED--------\nCtrl+c called\n");
		S.logClose();
		close(sockfd);
		exit(EXIT_SUCCESS);
	}
}		


/*
   FUNCTION NAME : main 
 
   DESCRIPTION	: Function to set up server configuration and calls 
 			      readRequest or writeRequest based on the client request.
 
   PARAMETERS	: void
 
   RETURN 		: EXIT_SUCCESS
 
*/

int main(void)
{
	struct addrinfo hints, *servinfo, *res;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char dst[INET6_ADDRSTRLEN];
	
	signal(SIGINT,signalHandler);

	// Configuration of server starts 
	memset(&hints, 0, sizeof hints);//sets the first count bytes of dest to the value c
	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; 
	
	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) 
	{
		S.logger("Server: getaddrinfo",'f',__func__,__LINE__);
		fprintf(stderr, "SERVER: getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// Loop through all the results and binding to the first 
	for(res = servinfo; res != NULL; res = res->ai_next) 
	{
		if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) //socket creation
		{
			S.logger("Server: socket",'w',__func__,__LINE__);
			perror("SERVER: socket");
			continue;
		}
		if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) 
		{
			S.logger("Server: socket",'w',__func__,__LINE__);
			close(sockfd);
			perror("SERVER: bind");
			continue;
		}
		break;
	}
	if (res == NULL) 
	{   S.logger("Server: failed to bind",'f',__func__,__LINE__);
    
		fprintf(stderr, "SERVER: failed to bind socket\n");
		return 2;
	}
	freeaddrinfo(servinfo);
	S.logOpen(LOGFILE);
	
	printf("SERVER: waiting to recvfrom...\n");
	// Configuration of server ends 
	

	//Main implementation starts 
	// Waiting for the first request from client - RRQ/WRQ 
	// Iterative server implementation 
	while(1)
	{
		memset(&their_addr,0,sizeof(their_addr));
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) 
		{
			S.logger("Server: recvfrom",'f',__func__,__LINE__);
			S.errorHandler(numbytes,"SERVER: recvfrom");
		}
		S.logger("Server got the packet",'i',__func__,__LINE__);
		printf("SERVER: got packet from %s\n", inet_ntop(their_addr.ss_family, S.getAddress((struct sockaddr *)&their_addr), dst, sizeof dst));
		printf("SERVER: packet is %d bytes long\n", numbytes);
		buf[numbytes] = '\0';
		printf("SERVER: packet contains \"%s\"\n", buf);
		
		/* Read request */
		S.logger("Server: Read request",'d',__func__,__LINE__);
		if(buf[0] == '0' && buf[1] == '1')
		{
			rv=S.readRequest(sockfd, buf, their_addr, addr_len, res);
			S.logMessage("Server received READ REQUEST(RRQ) from %s \n",inet_ntop(their_addr.ss_family, S.getAddress((struct sockaddr *)&their_addr), dst, sizeof dst));
			if(rv==EXIT_FAILURE)
			{
				S.logger("Server: Read request unsuccesful",'w',__func__,__LINE__);
				fprintf(stderr,"READ REQUEST UNSUCCESSFUL\n");
			}	
		}
		//Write request 
		else if (buf[0] == '0' && buf[1] == '2')
		{
			S.logger("Server: Write request",'d',__func__,__LINE__);
			rv=S.writeRequest(sockfd, buf, their_addr, addr_len);
			S.logMessage("Server received WRITE REQUEST(WRQ) from %s \n",inet_ntop(their_addr.ss_family, S.getAddress((struct sockaddr *)&their_addr), dst, sizeof dst));
			if(rv==EXIT_FAILURE)
			{
				S.logger("Server: Write request unsuccesful",'w',__func__,__LINE__);
				fprintf(stderr,"WRITE REQUEST UNSUCCESSFUL\n");
			}
		} 
		else 
		{
			fprintf(stderr,"INVALID REQUEST\n");
			S.logMessage("Server received an invalid request from %s \n",inet_ntop(their_addr.ss_family, S.getAddress((struct sockaddr *)&their_addr), dst, sizeof dst));
			S.logger("Server: Invalid request",'f',__func__,__LINE__);
		}
	}
	// Main implementation ends 
	return EXIT_SUCCESS;
}



