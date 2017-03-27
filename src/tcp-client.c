#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "init.h"

#define CHUNK 1024

static double timedifference_msec(struct timeval t0, struct timeval t1) {
		return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

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
	
	struct timeval start, end;
	double elapsed;
	
	file = fopen(filename, "r");
	
	gettimeofday(&start, 0);
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

			//printf("CHUNK %d : %s\n", chunkNumber++, buffer);
			memset(buffer, 0, CHUNK);
		}
	}
	
	write(sock, "FILE_TRANSFER_DONE",CHUNK);

	gettimeofday(&end, 0);
	elapsed = timedifference_msec(start, end)/1000;

	double throughput = (double)totalTrans*8/1024/1024/elapsed;
	printf("Total transfer : %lu bytes for %f secs\n", totalTrans, elapsed);
	printf("Throughput : %f Mbps\n", throughput);
	fclose(file);
}

void tcpClientRun(char *host, char *port, char *fileName) {
	int sock = initTCPClient(host, atoi(port));
	// connection handle
	fileHandle(sock, fileName);
	close(sock);
}
