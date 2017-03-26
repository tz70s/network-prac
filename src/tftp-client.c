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
#include <sys/time.h>

#define CHUNK 1024
#define SEQ_SIZE 20

double timedifference_msec(struct timeval t0, struct timeval t1) {
	return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

static void fileHandle(int sock, char *filename, struct sockaddr_in servAddr, socklen_t servAddrLen) {
	
	// initialized buffer
	
	uint8_t buffer[CHUNK];
	memset(buffer, 0, CHUNK);
	FILE *file;
	size_t nread;
	
	int statusCode = 0, chunkNumber = 1;
	
	long totalTrans = 0;
	long loss = 0;

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
	
	struct timeval start, end;
	double elapsed;

	struct timeval prev, current;
	double eachPack;
	
	unsigned long int totalsize = 0;
	
	gettimeofday(&start, 0);
	gettimeofday(&current, 0);
	if(file) {
	
		while ((nread = fread(buffer, 1, sizeof(buffer), file)) > 0) {
			totalsize += nread;
			prev = current;
			gettimeofday(&current, 0);
			eachPack = timedifference_msec(prev, current);

			// CHUNK transfer time
			
			printf("Transfer this CHUNK in %f\n ms", eachPack);

			while(1) {
				totalTrans++;
				//printf("Timestamp : %lu\n", current);
							
				if (reliableSender(sock, buffer, nread, seq, servAddr, servAddrLen) == 0) {
					printf("SEND SUCCESS\n");
					seq++;
					break;
				}
				loss++;
			}
			memset(buffer, 0, CHUNK);
		}
	}

	printf("Total transfer chunks: %lu, Total loss chunks: %lu, Loss rate: %f\n", totalTrans, loss, (float)loss / totalTrans);
	
	gettimeofday(&end, 0);
	elapsed = timedifference_msec(start, end)/1000;

	double throughput = (double)totalsize*8/1024/1024/elapsed;
	printf("Total transfer : %lu bytes for %f secs\n", totalsize, elapsed);
	printf("Throughput : %f Mbps\n", throughput);
	
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
