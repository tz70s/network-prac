#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "init.h"
#include "reliable-trans.h"

#define CHUNK 1024
#define SEQ_SIZE 20

static void fileHandle(int sock, char *filename, struct sockaddr_in servAddr, socklen_t servAddrLen) {
	
	// initialized buffer
	
	uint8_t buffer[CHUNK];
	memset(buffer, 0, CHUNK);
	FILE *file;
	size_t nread;
	
	int statusCode = 0, chunkNumber = 1;
	
	long totalTrans = 0;

	// transfer filename first
	
	while (1) {
	   	int ret = reliableWRQ(sock, filename, servAddr, servAddrLen);
		if (ret == 0) {
			break;
		}
	}
	
	// file open
	
	file = fopen(filename, "r");
	int seq = 1;

	if(file) {
	
		while ((nread = fread(buffer, 1, sizeof(buffer), file)) > 0) {
			
			while(1) {
				if (reliableSender(sock, buffer, nread, seq, servAddr, servAddrLen) == 0) {
					printf("SEND SUCCESS\n");
					seq++;
					break;
				}
			}
			memset(buffer, 0, CHUNK);
		}
	}
	fclose(file);
	
}

void udpClientRun(char *host, char *port, char *fileName) {
	
	int sock = initUDPClient();
	struct sockaddr_in servAddr = initUDPAddr(host, atoi(port));
	socklen_t servAddrLen = sizeof(servAddr);
	
	timeout(sock, 500000);
	// connection handle
	fileHandle(sock, fileName, servAddr, servAddrLen);
	close(sock);
}
