#pragma once 

#include <iostream>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <errno.h>
# include <string.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <time.h>


/*
 * 		MACROS
 */
# define MYPORT "4950" 	// port to be opened on server 
# define SERVERPORT "4950" 	// the port users will be connecting to 
# define MAXBUFLEN 550 	// get sockaddr, IPv4 or IPv6 
# define MAX_READ_LEN 512 	// maximum data size that can be sent on one packet 
# define MAX_FILENAME_LEN 100 // maximum length of file name supported 
# define MAX_PACKETS 99 	// maximum number of file packets 
# define MAX_TRIES 3 		// maximum number of tries if packet times out 
# define TIME_OUT 5 		// in seconds 
#define RRQ_OPCODE "01"	//opcode for read request
#define WRQ_OPCODE "02"	//opcode for write reqest
#define DATA_OPCODE "03"	//opcode for data packet
#define ACK_OPCODE "04"	//opcode for acknowledgement packet
#define ERR_OPCODE "05"	//opcode for error packet

using namespace std;

class Client{

int sockfd;

public:

	//methods

         void numberToString(char *str, int num);
         char* makeRRQ(char *filename);
         char* makeWRQ(char *filename);
         char* makeDataPacket(int block, char *data);
         char* makeACK(char* block);
         char* makeERR(char *errcode, char* errmsg);
         void *getAddress(struct sockaddr *sa);
        
	//loger methods
		 int logger(char* message, char logType,const char *funcName,int lineNo);
	 //socket methods
		 int conn();
         int maxTries(int sockfd,char *buf,struct sockaddr_storage *their_addr, socklen_t addr_len,struct addrinfo *res,char *last_message);
         int checkTimeout(int sockfd, char *buf, struct sockaddr_storage *their_addr, socklen_t addr_len);
         int readFile(int sockfd, struct sockaddr_storage their_addr,struct addrinfo *res, char *file,char *server);
         int writeFile(int sockfd, struct sockaddr_storage their_addr,struct addrinfo *res,char *file,char *server);
         void errorHandler(int ret, const char *mesg);
		

	private:

		
	struct addrinfo hints, *servinfo, *res;
	int rv;
	struct sockaddr_storage their_addr;

};