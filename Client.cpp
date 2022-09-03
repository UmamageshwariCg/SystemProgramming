
#include "Client.h"

/*
 **  FUNCTION NAME	: numberToString
 **
 **  DESCRIPTION	: It converts block number to string of lenght two
 **
 **  PARAMETERS	: char *temp, int num 
 **
 **  RETURN 		: void
 **
*/
void Client :: numberToString(char *temp, int num)
{
	if(num==0)
	{
		temp[0] = '0', temp[1] = '0', temp[2] = '\0';
	}
       	else if(num%10 > 0 && num/10 == 0){

		char ch = num+'0';
		temp[0] = '0', temp[1] = ch, temp[2] = '\0';
	} 
	else if(num%100 > 0 && num/100 == 0)
	{
		char charTwo = (num%10)+'0';
		char charOne = (num/10)+'0';
		temp[0] = charOne, temp[1] = charTwo, temp[2] = '\0';
	}
       	else 
	{
		temp[0] = '9', temp[1] = '9', temp[2] = '\0';
	}
}
/*
 **  FUNCTION NAME	: makeRRQ
 **
 **  DESCRIPTION	: It makes packet for read request
 **
 **  PARAMETERS	: char *filename
 **
 **  RETURN 		: packet  
 **
 */
char* Client :: makeRRQ(char *filename){
	char *packet;
	packet = (char*)malloc(2+strlen(filename)+1);
	size_t len=sizeof(packet);
	memset(packet, 0, len);
	strcat(packet, RRQ_OPCODE); /* opcode */
	strcat(packet, filename);
	return packet;
}

/*
 **  FUNCTION NAME	: makeWRQ
 **
 **  DESCRIPTION	: It  makes packet for write request
 **
 **  PARAMETERS	: char *filename 
 **
 **  RETURN 		: packet
 **
*/
char* Client :: makeWRQ(char *filename)
{
	char *packet;
	packet = (char*)malloc(2+strlen(filename)+1);
	size_t len=sizeof(packet);
	memset(packet, 0, len);
	strcat(packet, WRQ_OPCODE); // opcode 
	strcat(packet, filename);
	return packet;
}

/*
 **  FUNCTION NAME	: makeDataPacket
 **
 **  DESCRIPTION	: It  makes data packet
 **
 **  PARAMETERS	: int block, char *data 
 **
 **  RETURN 		: packet
*/
char* Client :: makeDataPacket(int block, char *data)
{
	char *packet;
	char temp[3];
	numberToString(temp, block);
	packet = (char*)malloc(4+strlen(data)+1);
	size_t len=sizeof(packet);
	memset(packet, 0, len);
	strcat(packet, DATA_OPCODE); // opcode 
	strcat(packet, temp);
	strcat(packet, data);
	return packet;
}

/*
 **  FUNCTION NAME	: makeACK 
 **
 **  DESCRIPTION	: It  makes ACK packet
 **
 **  PARAMETERS	: char *block
 **
 **  RETURN 		: packet
 **
*/
char* Client :: makeACK(char* block)
{
	char *packet=NULL;
	packet = (char *)malloc(2+strlen(block)+1);
	strcpy(packet, ACK_OPCODE); // opcode 
	strcat(packet, block);
	return packet;
}

/*
 **  FUNCTION NAME	: makeERR 
 **
 **  DESCRIPTION	: It  makes ERR packet
 **
 **  PARAMETERS	: char *errcode, char *errmsg
 **
 **  RETURN 		: packet
*/

char* Client :: makeERR(char *errcode, char* errmsg)
{
	char *packet;
	packet = (char*)malloc(4+strlen(errmsg)+1);
	size_t len=sizeof(packet);
	memset(packet, 0, len);
	strcat(packet, ERR_OPCODE); // opcode 
	strcat(packet, errcode);
	strcat(packet, errmsg);
	return packet;
}

/*
 **  FUNCTION NAME	: getAddress
 **
 **  DESCRIPTION	: it typecasts an unspecific address into IPv4 or IPv6
 **
 **  PARAMETERS	: struct sockaddr *sa 
 **
 **  RETURN 		: void
 **
*/
 
void *Client :: getAddress(struct sockaddr *sa){
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
 
/*
 **  FUNCTION NAME	: checkTimeout
 **
 **  DESCRIPTION	: Checks for timeout
 **
 **  PARAMETERS	: int sockfd, char *buf, struct sockaddr_storage *their_addr,
                         socklen_t addr_len
 **
 **  RETURN 		: -2 or -1 or number of bytes received
 **
*/
 int Client :: checkTimeout(int sockfd, char *buf, struct sockaddr_storage *their_addr, socklen_t addr_len)
{
	fd_set fds;
	int ret;
	struct timeval tv;

	// set up the file descriptor set 
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);

	// set up the struct timeval for the timeout 
	tv.tv_sec = TIME_OUT;
	tv.tv_usec = 0;

	// wait until timeout or data received 
	ret = select(sockfd+1, &fds, NULL, NULL, &tv);
	if (ret == 0)
	{
		printf("timeout\n");
		logger("Client: Timeout",'w',__func__,__LINE__);
		return -2; // timeout 
	}
       	else if (ret == -1)
	{
		printf("Select error\n");
		return -1; /* error */
	}
	return recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)their_addr, &addr_len);
}

/*
 **  FUNCTION NAME	: maxTries
 **
 **  DESCRIPTION	: The maximum number of tries the host will
 **			  try to send the packet to the other host
 **
 **  PARAMETERS	: int sockfd,char *buf,struct sockaddr_storage *their_addr,
                          socklen_t addr_len,struct addrinfo *res,char *last_message
 **
 **  RETURN 		: numBytes
 **
*/
 int Client :: maxTries(int sockfd,char *buf,struct sockaddr_storage *their_addr, socklen_t addr_len,struct addrinfo *res,char *last_message)
{
	logger("Max tries function",'d',__func__,__LINE__);
	int times;
	int numbytes;
	for(times=0;times<=MAX_TRIES;++times)
	{
		if(times == MAX_TRIES)
		{
			// Reached max no. of tries 
			logger("Client: Max number of tries reached",'w',__func__,__LINE__);
			printf("CLIENT: MAX NUMBER OF TRIES REACHED\n");
			exit(1);
		}
		// Checking if timeout has occurred or not 
		logger("Client: Timeout check",'d',__func__,__LINE__);
		numbytes = checkTimeout(sockfd, buf, their_addr, addr_len);
		if(numbytes == -1)
		{ 	
			// Error 
			logger("Client: select() ",'f',__func__,__LINE__);
			errorHandler(numbytes,"CLIENT: recvfrom");
		}
	       else if(numbytes == -2)
		{ 
			// Timeout 
			logger("Client: timeout",'w',__func__,__LINE__);
			printf("CLIENT: try no. %d\n", times+1);
			int temp_bytes;
			if((temp_bytes = sendto(sockfd, last_message, strlen(last_message), 0, res->ai_addr, res->ai_addrlen)) == -1)
			{
				logger("Client: sendto",'f',__func__,__LINE__);
				errorHandler(numbytes,"CLIENT ACK: sendto");
			}
			printf("CLIENT: sent %d bytes AGAIN\n", temp_bytes);
			logger("Client sent bytes again",'i',__func__,__LINE__);
			continue;
		}
	       else 
		{ 	
			//Valid 
			break;
		}
	}
	return numbytes;
 }
 

/*
 **  FUNCTION NAME	: readFile
 **
 **  DESCRIPTION	: Function to read file from server
 **
 **  PARAMETERS	: int sockfd, struct sockaddr_storage their_addr,struct addrinfo *res,
                          char *file,char *server
 **
 **  RETURN 		: EXIT_SUCCESS
 
*/
 
 int Client :: readFile(int sockfd, struct sockaddr_storage their_addr,struct addrinfo *res, char *file,char *server)
{
	char *message = makeRRQ(file);
	int numbytes;
	FILE *fp;
	char last_recv_message[MAXBUFLEN];
	char filename[MAX_FILENAME_LEN];
	char buf[MAXBUFLEN];
	char *block;
	char *t_msg;
	strcpy(last_recv_message, "");
	char last_sent_ack[10];
	char dst[INET6_ADDRSTRLEN];
	socklen_t addr_len;
	addr_len = sizeof their_addr;
	strcpy(last_sent_ack, "");
	if((numbytes = sendto(sockfd, message, strlen(message), 0, res->ai_addr, res->ai_addrlen)) == -1)
	{
		logger("Client: sendto",'f',__func__,__LINE__);
		errorHandler(numbytes,"CLIENT: sendto");
	}
	printf("CLIENT: sent %d bytes to %s\n", numbytes, server);
	logger("Client sent to server",'i',__func__,__LINE__);
	int flag=1;
	//Receiving actual file 
	logger("Receiving actual file",'d',__func__,__LINE__);
	int c_written;
	do
	{
		// Receiving file packet data 
		logger("Receiving file packet data",'i',__func__,__LINE__);
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
		{
			logger("Client: receivefrom",'f',__func__,__LINE__);
			errorHandler(numbytes,"CLIENT: recvfrom");
		}
		printf("CLIENT: got packet from %s\n", inet_ntop(their_addr.ss_family, getAddress((struct sockaddr *)&their_addr), dst, INET6_ADDRSTRLEN));
		logger("Client: got packet from",'i',__func__,__LINE__);
		printf("CLIENT: packet is %d bytes long\n", numbytes);
		buf[numbytes] = '\0';
		printf("CLIENT: packet contains \"%s\"\n", buf);

		// Checking if error packet 
		if(buf[0]=='0' && buf[1]=='5')
		{
			fprintf(stderr, "CLIENT: got error packet: %s\n", buf);
			logger("Client: got error packet",'w',__func__,__LINE__);
			free(message);
			exit(1);
		}
		if(flag==1)
		{	
			strcpy(filename, file);
			strcat(filename, "_client");
			fp = fopen(filename, "wb");
			if(fp == NULL)
			{ 	
				// Error checking 
				logger("Client: error opening file",'f',__func__,__LINE__);
				fprintf(stderr,"CLIENT: error opening file: %s\n", filename);
				free(message);
				exit(1);
			}
			flag=0;
		}
		// Sending last ack again as it was not reached 
		if(strcmp(buf, last_recv_message) == 0)
		{
			logger("Client: Last ack has not reached",'w',__func__,__LINE__);
			numbytes=sendto(sockfd, last_sent_ack, strlen(last_sent_ack), 0, (struct sockaddr *)&their_addr, addr_len);
			if(numbytes==-1){
				logger("Client: error in sendto system call",'f',__func__,__LINE__);
				errorHandler(numbytes,"sendto");
			}
			continue;
		}
		// Writing file packet data 
		c_written = strlen(buf+4);
		fwrite(buf+4, sizeof(char), c_written, fp);
		strcpy(last_recv_message, buf);

		// Sending acknowledgement packet data 
		logger("Client: Sending ack packet data",'d',__func__,__LINE__);
		block=(char*)malloc(3*sizeof(char));
		strncpy(block, buf+2, 2);
		block[2] = '\0';
		t_msg = makeACK(block);
		if((numbytes = sendto(sockfd, t_msg, strlen(t_msg), 0, res->ai_addr, res->ai_addrlen)) == -1)
		{
			logger("Client: failed to send acknowledment",'f',__func__,__LINE__);
			errorHandler(numbytes,"CLIENT ACK: sendto");
		}
		printf("CLIENT: sent %d bytes\n", numbytes);
		logger("Client sent data",'i',__func__,__LINE__);
		strcpy(last_sent_ack, t_msg);
		strcpy(buf,last_recv_message);
		free(block);
		free(t_msg);
	} 
	while(c_written == MAX_READ_LEN);
	printf("NEW FILE: %s SUCCESSFULLY MADE\n", filename);
	logger("New file successfully made",'i',__func__,__LINE__);
	free(message);
	fclose(fp);
	return EXIT_SUCCESS;
}

	
/*
 **  FUNCTION NAME	: writeFile
 **
 **  DESCRIPTION	: Function to write file to server
 **
 **  PARAMETERS	: int sockfd, struct sockaddr_storage their_addr,struct addrinfo *res,
                          char *file,char *server
 **
 **  RETURN 		: EXIT_SUCCESS
 */
 
 int Client ::  writeFile(int sockfd, struct sockaddr_storage their_addr,struct addrinfo *res, char *file,char *server)
{
	char *message = makeWRQ(file);
	char *last_message;
	char dst[INET6_ADDRSTRLEN];
	char buf[MAXBUFLEN];
	char *t_msg;
	int numbytes;
	if((numbytes = sendto(sockfd, message, strlen(message), 0, res->ai_addr, res->ai_addrlen)) == -1)
	{
		logger("Client: sendto",'f',__func__,__LINE__);
		errorHandler(numbytes,"CLIENT: sendto");
	}
	printf("CLIENT: sent %d bytes to %s\n", numbytes, server);
	logger("Client: send data to server",'i',__func__,__LINE__);
	last_message = message;
	// Waiting for acknowledgement WRQ
	socklen_t addr_len;
	addr_len = sizeof(their_addr);
	logger("Client: Max tries function call",'d',__func__,__LINE__);
	numbytes=maxTries(sockfd,buf, &their_addr,addr_len,res,last_message);
	printf("CLIENT: got packet from %s\n", inet_ntop(their_addr.ss_family, getAddress((struct sockaddr *)&their_addr), dst, INET6_ADDRSTRLEN));
	printf("CLIENT: packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("CLIENT: packet contains \"%s\"\n", buf);
	if(buf[0]=='0' && buf[1]=='4')
	{
		FILE *fp = fopen(file, "rb");
		if(fp == NULL || access(file, F_OK) == -1)
		{
			fprintf(stderr,"CLIENT: file %s does not exist\n", file);
			char *e_msg = makeERR("01", "ERROR_FILE_NOT_FOUND");
			printf("%s\n", e_msg);
			sendto(sockfd, e_msg, strlen(e_msg), 0, res->ai_addr, res->ai_addrlen);
			free(e_msg);
			free(message);
			logger("Client: File does not exist",'w',__func__,__LINE__);
			exit(1);
		}
		// Calculating of size of file 
		logger("Calculating the size of file",'d',__func__,__LINE__);
		int block = 1;
		fseek(fp, 0, SEEK_END);
		int total = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		int remaining = total;
		if(remaining == 0)
			++remaining;
		else if(remaining%MAX_READ_LEN == 0)
			--remaining;

		logger("Reading file data packet",'i',__func__,__LINE__);
		while(remaining>0)
		{
			// Reading file data packet 
			char temp[MAX_READ_LEN+5];
			if(remaining>MAX_READ_LEN)
			{
				fread(temp, MAX_READ_LEN, sizeof(char), fp);
				temp[MAX_READ_LEN] = '\0';
				remaining -= (MAX_READ_LEN);
			}
		       	else 
			{
				fread(temp, remaining, sizeof(char), fp);
				temp[remaining] = '\0';
				remaining = 0;
			}
			// Sending file data packet 
			logger("Sending file data packet",'d',__func__,__LINE__);
			t_msg = makeDataPacket(block, temp);
			if((numbytes = sendto(sockfd, t_msg, strlen(t_msg), 0, res->ai_addr, res->ai_addrlen)) == -1)
			{
				logger("Client: sendto",'f',__func__,__LINE__);
				errorHandler(numbytes,"CLIENT: sendto");
			}
			printf("CLIENT: sent %d bytes to %s\n", numbytes, server);
			logger("Client: sent bytes to server",'i',__func__,__LINE__);
			last_message = t_msg;
			//s Waiting for acknowledgement data packet 
			numbytes=maxTries(sockfd,buf, &their_addr,addr_len,res,last_message);
			printf("CLIENT: got packet from %s\n", inet_ntop(their_addr.ss_family, getAddress((struct sockaddr *)&their_addr), dst, INET6_ADDRSTRLEN)); 
			logger("Client got packet",'i',__func__,__LINE__);
			printf("CLIENT: packet is %d bytes long\n", numbytes);
			buf[numbytes] = '\0';
			printf("CLIENT: packet contains \"%s\"\n", buf);

			if(buf[0]=='0' && buf[1]=='5')
			{
				logger("Client: error packet received",'w',__func__,__LINE__);
				// If error packet received 
				fprintf(stderr, "CLIENT: got error packet: %s\n", buf);
				//free(last_message);
				free(t_msg);
				free(message);
				fclose(fp);
				exit(1);
			}
			++block;
			if(block>MAX_PACKETS)
				block = 1;
			free(t_msg);
		}
			free(message);
			fclose(fp);
		}
	else 
	{
		// Some bad packed received 
		logger("Client: ack expecting but got",'w',__func__,__LINE__);
		fprintf(stderr,"CLIENT ACK: expecting but got: %s\n", buf);
		exit(1);
	}
	return EXIT_SUCCESS;
}

/*
 **  FUNCTION NAME	: logger
 **
 **  DESCRIPTION	: Used to store 4 types of log messages to their  
 ** 			  respective files
 **
 **  PARAMETERS	: char* message, char logType
 **
 **  RETURN 		: EXIT_SUCCESS
*/
int Client :: logger(char* message, char logType,const char *funcName,int lineNo)
{
	/* info - i
	   fatal - f
	   warning - w
	   debug - d
	*/
	char LOG_PATH[]="../logs";
	time_t ltime = time(NULL);
	struct tm res;
	char TIMESTAMP[32];
	localtime_r(&ltime,&res);
	asctime_r(&res,TIMESTAMP);
	FILE *logfile;
	char logFileName[100];
    	switch(logType)
    	{
		case 'i':
		    sprintf(logFileName,"%s/info.log",LOG_PATH);
		    logfile = fopen(logFileName,"a+");
		    fprintf(logfile,"\n~~%s[%s : %s : %d]\t%s\n----------\n",TIMESTAMP,__FILE__, funcName,lineNo , message);
		    fclose(logfile);
		    break;
		case 'f':
		    sprintf(logFileName,"%s/fatal.log",LOG_PATH);
		    logfile = fopen(logFileName,"a+");
		    fprintf(logfile,"\n~~%s[%s : %s : %d]\t%s\n----------\n",TIMESTAMP,__FILE__, funcName,lineNo, message);
		    fclose(logfile);
		    break;
		case 'w':
		    sprintf(logFileName,"%s/warnings.log",LOG_PATH);
		    logfile = fopen(logFileName,"a+");
		    fprintf(logfile,"\n~~%s[%s : %s : %d]\t%s\n----------\n",TIMESTAMP,__FILE__, funcName,lineNo, message);
		    fclose(logfile);
		    break;
		case 'd':
		    sprintf(logFileName,"%s/debug.log",LOG_PATH);
		    logfile = fopen(logFileName,"a+");
		    fprintf(logfile,"\n~~%s[%s : %s :%d]\t%s\n----------\n",TIMESTAMP,__FILE__, funcName,lineNo, message);
		    fclose(logfile);
		    break;
    	}
    	return EXIT_SUCCESS;
}
/*
 **  FUNCTION NAME	: errorHandler
 **
 **  DESCRIPTION	: Handles the error occured due to failure of 
 ** 			  system call or library function 
 **			  
 **  PARAMETERS	: int ret, const char *mesg
 **
 **  RETURN 		: void
 **
 */
void Client :: errorHandler(int ret, const char *mesg)
{
	if(ret == -1)
	{
		perror(mesg);
		exit(EXIT_FAILURE);
	}
}
 
