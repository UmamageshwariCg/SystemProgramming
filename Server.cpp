#include <Server.h>

static FILE *logFptr;

/*
  FUNCTION NAME	: numberToString
 
  DESCRIPTION	: Converts block number to string of length two
 
  PARAMETERS  	: char *temp, int num
 
  RETURN 		: void

  */
void Server :: numberToString(char *temp, int num)
{
	if(num==0)
	{
		temp[0] = '0', temp[1] = '0', temp[2] = '\0';
	} 
	else if(num%10 > 0 && num/10 == 0)
	{
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
 FUNCTION NAME	: makeDataPacket
 
  DESCRIPTION	: It makes data packet
 
   PARAMETERS	: int block, char *data
 
   RETURN 		: packet
 
 */

char* Server :: makeDataPacket(int block, char *data){
	char *packet=NULL;
	char temp[3];
	numberToString(temp, block);
	packet = (char*)malloc(4+strlen(data)+1);
	strcpy(packet, DATA_OPCODE);//opcode
	strcat(packet, temp);
	strcat(packet, data);
	return packet;
}

/*
  FUNCTION NAME	: makeACK
 
  DESCRIPTION	: It makes acknowledgement packet

  PARAMETERS	: char* block
 
  RETURN 		: packet
 
 */

char* Server :: makeACK(char* block){
	char *packet=NULL;
	packet = (char*)malloc(2+strlen(block)+1);
	strcpy(packet, ACK_OPCODE);//opcode
	strcat(packet, block); 
	return packet;
}

/*
   FUNCTION NAME	: makeERR
 
  DESCRIPTION	: It makes error packet 
 
  PARAMETERS	: char *errcode, char* errmsg
 
  RETURN 		: packet
 */

char* Server :: makeERR(char *errcode, char* errmsg){
	char *packet=NULL;
	packet = (char*)malloc(4+strlen(errmsg)+1);
	size_t len=sizeof(packet);
	memset(packet, 0, len);
	strcat(packet, ERR_OPCODE);//opcode
	strcat(packet, errcode);
	strcat(packet, errmsg);
	return packet;
} 

/*
  FUNCTION NAME	: logMessage
 
  DESCRIPTION	: Maintaining  a log file to store information about all
 			  requests from client
 
  PARAMETERS 	: const char *format, ....
 
  RETURN 		: void
 
 */

void Server::logMessage(const char *format,...)
{
	va_list argList;
	const char *TIMEFORMAT = "%d-%m-%Y %X";
	#define DATESIZE sizeof("DD-MM-YYYY HH:MM:SS")
	char timeStamp[DATESIZE];
	time_t t;
	struct tm *tmVar;
	t = time(NULL);
	tmVar = localtime(&t);
	if(tmVar == NULL || strftime(timeStamp,DATESIZE,TIMEFORMAT,tmVar) == 0)
	{
		fprintf(logFptr, "***Unknown time***: ");
	}
	else
	{
		fprintf(logFptr, "%s: ",timeStamp);
	}
	va_start(argList, format);
	vfprintf(logFptr, format, argList);
	fprintf(logFptr,"\n");
	va_end(argList);

}

/*
   FUNCTION NAME	: logOpen
 
 DESCRIPTION	: Function to open the log file
 
  PARAMETERS	: const char *logFileName
 
  RETURN 		: void
 
 */
void Server :: logOpen(const char *logFileName)
{
	mode_t oldPerms;
	oldPerms = umask(077);
	logFptr = fopen(logFileName, "a");
	umask(oldPerms);
	if(logFptr == NULL)
	{
		perror("logfile open");
		exit(EXIT_FAILURE);
	}
	setbuf(logFptr, NULL);
	logMessage("opened log file");

}

/*
   FUNCTION NAME	: logClose
 
   DESCRIPTION	: Function to close the log file
 
  PARAMETERS	: void
 
   RETURN 		: void
 
*/
void Server :: logClose(void)
{
	logMessage("closing log file");
	fclose(logFptr);
}

/*
   FUNCTION NAME	: getAddress
 
   DESCRIPTION	: it typecasts an unspecific address into IPv4 or IPv6
 
   PARAMETERS	: struct sockaddr *sa
 
   RETURN 		: void
 
*/
void* Server :: getAddress(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) 
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*
   FUNCTION NAME	: checkTimeout
 
   DESCRIPTION	: Checks for the timeout condition
 
  PARAMETERS	: int sockfd, char *buf, struct sockaddr_storage their_addr, socklen_t addr_len
 
   RETURN 		: -2 or -1 or number of bytes received
 
*/
int Server :: checkTimeout(int sockfd, char *buf, struct sockaddr_storage their_addr, socklen_t addr_len){
	fd_set fds;
	int ret;
	struct timeval tv;

	// Setting up the file descriptor set 
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);

	// Setting up the struct timeval for the timeout 
	tv.tv_sec = TIME_OUT;
	tv.tv_usec = 0;

	// Waiting until timeout or data received 
	ret = select(sockfd+1, &fds, NULL, NULL, &tv);
	if (ret == 0)
	{
		printf("timeout\n");
		logger("Server: Timeout",'w',__func__,__LINE__);
		return -2; // Timeout 
	} 
	else if (ret == -1)
	{
		printf("error\n");
		return -1; // Error 
	}
	return recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
}

/*
  FUNCTION NAME	: maxTries
 
   DESCRIPTION	: The maximum number of tries the host will
 			  try to send the packet to the other host
 
   PARAMETERS	: int sockfd, char *buf, struct sockaddr_storage their_addr, 
                          socklen_t addr_len, struct addrinfo *res, char *t_msg
 
   RETURN 		: numbytes
 
 */
int Server :: maxTries(int sockfd, char *buf, struct sockaddr_storage their_addr, socklen_t addr_len, struct addrinfo *res, char *t_msg){
	int times;
	int numbytes;
	for(times=0;times<=MAX_TRIES;++times)
	{
		if(times == MAX_TRIES)
		{
			printf("SERVER: MAX NUMBER OF TRIES REACHED\n");
			logger("Server: Max number of tries reached",'f',__func__,__LINE__);
			return -1;
		}
		numbytes = checkTimeout(sockfd, buf, their_addr, addr_len);
		if(numbytes == -1)
		{	/* Error */
			logger("Server: recvfrom",'f',__func__,__LINE__);
			errorHandler(numbytes,"SERVER: recvfrom");
		} 
		else if(numbytes == -2)
		{	/* Timeout */
			printf("SERVER: try no. %d\n", times+1);
			int temp_bytes;
			if((temp_bytes = sendto(sockfd, t_msg, strlen(t_msg), 0, res->ai_addr, res->ai_addrlen)) == -1)
			{
				logger("Server: sendto",'f',__func__,__LINE__);
				errorHandler(temp_bytes,"SERVER: ACK: sendto");
			}
			printf("SERVER: sent %d bytes AGAIN\n", temp_bytes);
			logger("Server: sent bytes again",'i',__func__,__LINE__);
			continue;
		} 
		else 
		{ 
			// Valid condition 
			break;
		}
	}
	return numbytes;
}
/*
   FUNCTION NAME	: readRequest
 
   DESCRIPTION	: Funtion to serve client read request.It checks for the file in server
                       and sends data to client.Each data packet contains 512 bytes of data.
 			          Final data packet contains data less than 512 bytes. This indicates 
  			          connection termination.
 
   PARAMETERS	: int sockfd, char *buf, struct sockaddr_storage their_addr,
                          socklen_t addr_len, struct addrinfo *res
 
   RETURN 		: EXIT_SUCCESS or EXIT_FAILURE
 
*/
int Server :: readRequest(int sockfd, char *buf, struct sockaddr_storage their_addr, socklen_t addr_len, struct addrinfo *res)
{
		char filename[MAX_FILENAME_LEN];
		int numbytes;
		char *t_msg;
		char dst[INET6_ADDRSTRLEN];
		strcpy(filename, buf+2);
		
		FILE *fp = fopen(filename, "rb"); //open in binary mode for reading 
		if(fp == NULL || access(filename, F_OK) == -1)
		{ 	
			// Sending error packet - File not found 
			fprintf(stderr,"SERVER: file '%s' does not exist, sending error packet\n", filename);
			char *e_msg = makeERR("01", "ERROR_FILE_NOT_FOUND");
			logger("Server: Error file not found",'w',__func__,__LINE__);
			printf("%s\n", e_msg);
			logger("Server: Sendto()",'d',__func__,__LINE__);
			sendto(sockfd, e_msg, strlen(e_msg), 0, (struct sockaddr *)&their_addr, addr_len);
			free(e_msg);
			return EXIT_FAILURE;
		}
		
		// Starting to send file 
		logger("Starting to send file",'d',__func__,__LINE__);
		int block = 1;
		fseek(fp, 0, SEEK_END);    //sets the file position at the offset 
		int total = ftell(fp);    //tells the current position of the file pointer in file 
		fseek(fp, 0, SEEK_SET);
		int remaining = total;
		if(remaining == 0)
			++remaining;
		else if(remaining%MAX_READ_LEN == 0)
			--remaining;
		while(remaining>0)
		{
			// Reading the file 
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
			// Sending data packet
			t_msg = makeDataPacket(block, temp);
			if((numbytes = sendto(sockfd, t_msg, strlen(t_msg), 0, (struct sockaddr *)&their_addr, addr_len)) == -1)
			{
				logger("Server: ACK sendto",'f',__func__,__LINE__);
				errorHandler(numbytes,"SERVER ACK: sendto");
			}
			printf("SERVER: sent %d bytes\n", numbytes);

			// Waiting for acknowledgement - Data packet 
			// Checks the maxTries func 
			logger("Server: Maxtries",'d',__func__,__LINE__);
			numbytes=maxTries(sockfd, buf, their_addr, addr_len, res, t_msg);
			if(numbytes==-1)
			{
				logger("Server: failure",'w',__func__,__LINE__);
				free(t_msg);
				fclose(fp);
				return EXIT_FAILURE;
			}
			logger("Server: Got packet from client",'i',__func__,__LINE__);
			printf("SERVER: got packet from %s\n", inet_ntop(their_addr.ss_family, getAddress((struct sockaddr *)&their_addr), dst, sizeof dst));
			printf("SERVER: packet is %d bytes long\n", numbytes);
			buf[numbytes] = '\0';
			printf("SERVER: packet contains \"%s\"\n", buf);				
			++block;
			if(block>MAX_PACKETS)
				block = 1;
		free(t_msg);
		}
		fclose(fp);
		return EXIT_SUCCESS;
}

/*
   FUNCTION NAME	: writeRequest
 
   DESCRIPTION	: Funtion to serve client write request.Client will send data to server.
  			  If the received data packet is not an error, then server will create a file
 			  in write mode and place all the data received from client into the file
 
   PARAMETERS	: int sockfd, char *buf, struct sockaddr_storage their_addr,
                          socklen_t addr_len
 
   RETURN 		: EXIT_SUCCESS or EXIT_FAILURE
 
*/
int Server :: writeRequest(int sockfd, char *buf, struct sockaddr_storage their_addr, socklen_t addr_len)
{
	char *message = makeACK(WRQ_ACK);
	int numbytes;
	int flag=1;
	char *t_msg=NULL;
	FILE *fp;
	char dst[INET6_ADDRSTRLEN];
	char filename[MAX_FILENAME_LEN];
	char last_recv_message[MAXBUFLEN];
	strcpy(last_recv_message, buf);
	char last_sent_ack[10];
	strcpy(last_sent_ack, message);
	if((numbytes = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&their_addr, addr_len)) == -1)
	{
		logger("Server: ACK sendto",'f',__func__,__LINE__);
		errorHandler(numbytes,"SERVER ACK: sendto");
	}
	strcpy(filename, buf+2);
	strcat(filename, "_server");
	int c_written;
	do
	{
		// Receiving packet data 
		logger("Server: Receiving packet data",'d',__func__,__LINE__);
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) 
		{
			logger("Server: recvfrom",'f',__func__,__LINE__);
			errorHandler(numbytes,"SERVER: recvfrom");
		}
		logger("Server got packet from",'i',__func__,__LINE__);
		printf("SERVER: got packet from %s\n", inet_ntop(their_addr.ss_family, getAddress((struct sockaddr *)&their_addr), dst, sizeof dst));
		printf("SERVER: packet is %d bytes long\n", numbytes);
		buf[numbytes] = '\0';
		printf("SERVER: packet contains \"%s\"\n", buf);
		
		if(flag==1)
		{
			if(buf[0]=='0' && buf[1]=='5')
			{
				fprintf(stderr, "SERVER: got error packet: %s\n", buf);
				logger("Server: got error packet",'w',__func__,__LINE__);
				free(message);
				return EXIT_FAILURE;
			}
			if(access(filename, F_OK) != -1)
			{ 
				/* Sending error packet duplicate file */
				logger("Server: Sending error packet",'d',__func__,__LINE__);
				fprintf(stderr,"SERVER: file %s already exists, sending error packet\n", filename);
				char *e_msg = makeERR("06", "ERROR_FILE_ALREADY_EXISTS");
				if((numbytes=sendto(sockfd, e_msg, strlen(e_msg), 0, (struct sockaddr *)&their_addr, addr_len))==-1){
					logger("Server: sendto system call failed",'f',__func__,__LINE__);
					errorHandler(numbytes,"SERVER ERROR MESSAGE: sendto");
				}
				free(e_msg);
				free(message);
				return EXIT_FAILURE;
			}
			fp = fopen(filename, "wb");
			if(fp == NULL || access(filename, W_OK) == -1)
			{ 
				/* Sending error packet access denied */
				logger("Server: Access denied",'w',__func__,__LINE__);
				fprintf(stderr,"SERVER: file %s access denied, sending error packet\n", filename);
				char *e_msg = makeERR("02", "ERROR_ACCESS_DENIED");
				logger("Server: sendto",'d',__func__,__LINE__);
				if((numbytes=sendto(sockfd, e_msg, strlen(e_msg), 0, (struct sockaddr *)&their_addr, addr_len))==-1){
					logger("Server: sendto system call failed",'f',__func__,__LINE__);
					errorHandler(numbytes,"SERVER ERROR MESSAGE: sendto");
				}
				free(e_msg);
				free(message);
				fclose(fp);
				return EXIT_FAILURE;
			}
			flag=0;
		}
		// Sending last acknowledgement again as it has not reached 
		if(strcmp(buf, last_recv_message) == 0)
		{
			logger("Server: last ack sending",'i',__func__,__LINE__);
			if((numbytes=sendto(sockfd, last_sent_ack, strlen(last_sent_ack), 0, (struct sockaddr *)&their_addr, addr_len))==-1){
				logger("Server: sendto system call failed",'f',__func__,__LINE__);
				errorHandler(numbytes,"SERVER LAST SENT ACK: sendto");
			}
			continue;
		}
		// Writing file 
		c_written = strlen(buf+4);
		fwrite(buf+4, sizeof(char), c_written, fp);
		strcpy(last_recv_message, buf);

		// Sending acknowledgement packet data 
		char block[3];
		strncpy(block, buf+2, 2);
		block[2] = '\0';
		t_msg = makeACK(block);
		if((numbytes = sendto(sockfd, t_msg, strlen(t_msg), 0, (struct sockaddr *)&their_addr, addr_len)) == -1)
		{
			logger("Server: ack sendto",'f',__func__,__LINE__);
			errorHandler(numbytes,"SERVER ACK: sendto");
		}
		logger("Server sent to client",'i',__func__,__LINE__);
		printf("SERVER: sent %d bytes\n", numbytes);
		strcpy(last_sent_ack, t_msg);
		free(t_msg);
	}
	while(c_written == MAX_READ_LEN);
	logger("Server: New file made",'i',__func__,__LINE__);
	printf("NEW FILE: %s SUCCESSFULLY MADE\n", filename);
	free(message);
	fclose(fp);
	return EXIT_SUCCESS;
}

/*
   FUNCTION NAME	: logger
 
   DESCRIPTION	: Used to store 4 types of log messages to their  
  			      respective files
 
  PARAMETERS	: char* message, char logType
 
   RETURN 		: EXIT_SUCCESS
 
*/
int Server :: logger(char* message, char logType, const char *funcName,int lineNo)
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
		    fprintf(logfile,"~~%s[%s : %s : %d]\t%s\n----------\n",TIMESTAMP,__FILE__,funcName, lineNo, message);
		    fclose(logfile);
		    break;
		case 'f':
		    sprintf(logFileName,"%s/fatal.log",LOG_PATH);
		    logfile = fopen(logFileName,"a+");
		    fprintf(logfile,"~~%s[%s : %s : %d]\t%s\n----------\n",TIMESTAMP,__FILE__, funcName,lineNo, message);
		    fclose(logfile);
		    break;
		case 'w':
		    sprintf(logFileName,"%s/warnings.log",LOG_PATH);
		    logfile = fopen(logFileName,"a+");
		    fprintf(logfile,"~~%s[%s : %s : %d]\t%s\n----------\n",TIMESTAMP,__FILE__,funcName, lineNo, message);
		    fclose(logfile);
		    break;
		case 'd':
		    sprintf(logFileName,"%s/debug.log",LOG_PATH);
		    logfile = fopen(logFileName,"a+");
		    fprintf(logfile,"~~%s[%s : %s :%d]\t%s\n----------\n",TIMESTAMP,__FILE__, funcName,lineNo, message);
		    fclose(logfile);
		    break;
		
	}
	return EXIT_SUCCESS;
}

/*
   FUNCTION NAME	: errorHandler
 
   DESCRIPTION	: Handles the error occured due to failure of 
  			  system call or library function 
 
   PARAMETERS	: int ret, const char *mesg
 
   RETURN 		: void
 
*/
void Server :: errorHandler(int ret, const char *mesg)
{
	if(ret == -1)
	{
		perror(mesg);
		exit(EXIT_FAILURE);
	}
}



