#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "init.h"
#include "reliable-trans.h"
#define CHUNK 1024
#define SEQ_SIZE 20

static void fileHandle(int servSock, struct sockaddr_in clientAddr, socklen_t clientAddrSize) {

	// initialized receive buffer
	uint8_t buffer[CHUNK+SEQ_SIZE+3];
	memset(buffer, 0, CHUNK+SEQ_SIZE+3);
	
	// WRQ HANDLING
	
	while(1) {
		int ret = reliableReceiver(servSock, clientAddr, clientAddrSize, buffer);
		if (ret == 1) {
			printf("FILE_NAME : %s\n", buffer);
			break;
		}
	}

	/*
	int retCode = recvfrom(servSock, buffer, CHUNK, 0, (struct sockaddr *) &clientAddr, &clientAddrSize);
	errorMsg(retCode, "recv file name");

	FILE *f = fopen(buffer, "w");
	if (!f) {
		perror("ERROR openning file\n");
		exit(1);
	}

	while(1) {
		
		retCode = recvfrom(servSock, buffer, CHUNK, 0, (struct sockaddr *) &clientAddr, &clientAddrSize);
		errorMsg(retCode, "recv file");
		
		// FIN
		if (strcmp(buffer, "FILE_TRANSFER_DONE") == 0) {
			printf("FINISHED\n");
			break;
		}

		if ( fwrite(buffer, 1, retCode, f) < 0) {
			perror("ERROR writing file");
			exit(1);
		}
		memset(buffer, 0, CHUNK);
		
		retCode = sendto(servSock, "RESPONSE", 8, 0, (struct sockaddr*) &clientAddr, clientAddrSize);
		errorMsg(retCode, "response error");
	}

	fclose(f);
	*/
}

void udpServerRun(char *port) {
	
	// initialized
	int servSock = initUDPServer(atoi(port));

	// create client sock addr
	struct sockaddr_in clientAddr;
	
	// initialized
	memset(&clientAddr, 0, sizeof(clientAddr));
	
	socklen_t clientAddrSize = sizeof(clientAddr);

	fileHandle(servSock, clientAddr, clientAddrSize);

	close(servSock);
}


