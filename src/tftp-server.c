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
	// get the file name

	_RET_ rett;

	while(1) {
		rett = reliableReceiver(servSock, clientAddr, clientAddrSize, buffer);
		if (rett.type == 1) {
			printf("FILE_NAME : %s\n", buffer);
			break;
		}
	}

	// start handling file
	// seq check
	
	int chunk_count = 1;

	// create/open local file
	
	FILE *f = fopen(buffer, "w");
	if (!f) {
		perror("ERROR openning file\n");
		exit(1);
	}

	int payload_size = 0;
	
	while(1) {
		
		//sleep(0.3);
		rett = reliableReceiver(servSock, clientAddr, clientAddrSize, buffer);
		
		if (rett.type == 0) {
			printf("Packet loss maybe\n");
		} else if (rett.type == 1) {
			printf("ERROR OCCURED\n");
		} else if (rett.type == 2) {
			if (rett.seq != chunk_count) {
				continue;
			}
			printf("Write data to file\n");
			if ( fwrite(buffer, 1, CHUNK, f) < 0) {
				perror("ERROR writing file");
				exit(1);
			}
			chunk_count++;
		} else if (rett.type == 3) {
			printf("END\n");
			
			if ( fwrite(buffer, 1, rett.seq, f) < 0) {
				perror("ERROR writing file");
				exit(1);
			}
			chunk_count++;
			break;
		}

		memset(buffer, 0, CHUNK+SEQ_SIZE+3);
	}

	fclose(f);
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


