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

#define CHUNK 1024

static void fileHandle(int sock, char *filename) {
	
	// initialized buffer
	
	uint8_t buffer[CHUNK];
	memset(buffer, 0, CHUNK);
	FILE *file;
	size_t nread;
	
	int statusCode = 0, chunkNumber = 1;
	
	long int totalTrans = 0;
	
	// transfer file name first
	
	statusCode = write(sock, filename, CHUNK);
	if (statusCode < 0) {
		perror("ERROR writing to socket (filename)");
		exit(1);
	}

	// start transfer file
	
	file = fopen(filename, "r");
	if(file) {
		while ((nread = fread(buffer, 1, sizeof(buffer), file)) > 0) {
			statusCode = write(sock, buffer, nread);
			if ( statusCode < 0 ) {
				perror("ERROR writing to socket");
				exit(1);
			}

			totalTrans += statusCode;
			// reset 
			memset(buffer, 0, CHUNK);
			
			statusCode = read(sock, buffer, 8);
			
			if ( statusCode < 0 ) {
				perror("ERROR reading from socket");
				exit(1);
			}
			printf("CHUNK %d : %s\n", chunkNumber++, buffer);
			memset(buffer, 0, CHUNK);
		}
	}
	printf("TOTAL TRANS SIZE : %lu\n", totalTrans);
	write(sock, "FILE_TRANSFER_DONE",CHUNK);
}

void tcpClientRun(char *host, char *port, char *fileName) {
	int sock = initTCPClient(host, atoi(port));
	// connection handle
	fileHandle(sock, fileName);
	close(sock);
}
