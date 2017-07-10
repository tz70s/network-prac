#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "init.h"

#define CHUNK 1024

static void tcpFileHandle(int clientSock) {

	// initialized receive buffer
	uint8_t buffer[CHUNK];
	memset(buffer, 0, CHUNK);
	
	// get the file name
	
	int filelength = read(clientSock, buffer, CHUNK);
	if (filelength < 0) {
		perror("ERROR rading from socket (filelength)");
		exit(1);
	}

	FILE *f = fopen(buffer, "w");
	if (!f) {
		perror("ERROR openning file\n");
		exit(1);
	}

	memset(buffer, 0, CHUNK);

	long int totalRecvSize = 0;
	
	while(1) {
		
		int retCode = read(clientSock, buffer, CHUNK);
	
		if (retCode < 0) {
			perror("ERROR reading from socket");
			exit(1);
		}
		
		// FIN
		if (strcmp(buffer, "FILE_TRANSFER_DONE") == 0) {
			printf("FINISHED\n");
			break;
		}
		
		totalRecvSize += retCode;

		if ( fwrite(buffer, 1, retCode, f) < 0) {
			perror("ERROR writing file");
			exit(1);
		}
		
		memset(buffer, 0, CHUNK);
		
		
		retCode = write(clientSock, "RESPONSE", 8);
	
		if (retCode < 0) {
			perror("ERROR writing to socket");
			exit(1);
		}
		
	}

	fclose(f);
	printf("TOTAL RECV SIZE : %lu\n",totalRecvSize);
}

void tcpServerRun(char *port) {
	
	// initialized
	int servSock = initTCPServer(atoi(port));

	// listen for connection
	listen(servSock, 20);
	
	// create client sock addr
	struct sockaddr_in clientAddr;
	
	// initialized
	memset(&clientAddr, 0, sizeof(clientAddr));
	
	socklen_t clientAddrSize = sizeof(clientAddr);
	
	// accept
	int clientSock = accept(servSock, (struct sockaddr*)&clientAddr, &clientAddrSize);

	if (clientSock < 0) {
		perror("ERROR on accept");
		exit(1);
	}

	tcpFileHandle(clientSock);

	close(clientSock);

	close(servSock);
}


