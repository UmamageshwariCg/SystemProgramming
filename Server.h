
#pragma once 

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>


/*
  		MACROS	
*/

#define MYPORT "4950" // Port to be opened on server 
#define SERVERPORT "4950" // The port users will be connecting to 
#define MAXBUFLEN 550 // To get sockaddr, IPv4 or IPv6 
#define MAX_READ_LEN 512 // Maximum data size that can be sent on one packet 
#define MAX_FILENAME_LEN 100 // Maximum length of file name supported 
#define MAX_PACKETS 99 // Maximum number of file packets 
#define MAX_TRIES 3 // Maximum number of tries if packet times out 
#define TIME_OUT 5 // in seconds 
#define DATA_OPCODE "03"  //opcode for data packet
#define ACK_OPCODE "04"   //opcode for acknowledgement packet
#define ERR_OPCODE "05"   //opcode for error packet
#define WRQ_ACK "00"      //Acknowledgement block for write request



using namespace std;

class Server{

int sockfd;

public:

	//methods

                void numberToString(char *temp, int num);
      	        char* makeDataPacket(int block, char *data);
                char* makeACK(char* block);
		char* makeERR(char *errcode, char* errmsg);
        
		//loger methods
		void logMessage(const char *format,...);
		void logOpen(const char *logFileName);
		void logClose(void);
		int logger(char* message, char logType, const char *funcName,int lineNo);

	//socket methods
		int conn();
		void *getAddress(struct sockaddr *sa);
		int checkTimeout(int sockfd, char *buf, struct sockaddr_storage their_addr, socklen_t addr_len);
		int maxTries(int sockfd, char *buf, struct sockaddr_storage their_addr, socklen_t addr_len, struct addrinfo *res, char *t_msg);
		int readRequest(int sockfd, char *buf, struct sockaddr_storage their_addr, socklen_t addr_len, struct addrinfo *res);
		int writeRequest(int sockfd, char *buf, struct sockaddr_storage their_addr, socklen_t addr_len);
		void errorHandler(int ret, const char *mesg);

	private:

		struct addrinfo hints, *servinfo, *res;
		int rv;
		int numbytes;
		struct sockaddr_storage their_addr;
		char buf[MAXBUFLEN];
		socklen_t addr_len;
		char dst[INET6_ADDRSTRLEN];

};