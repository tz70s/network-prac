#ifndef RELIABLE_TRANS_H
#define RELIABLE_TRANS_H

//#include "init.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define CHUNK 1024
#define SEQ_SIZE 20
#define WRITE_FILE 3

static inline int getSeq(char * seqString, size_t len) {
	size_t i;
	
	char seq[SEQ_SIZE];

	for (i = 0; i < len; i++) {
		if (seqString[i] == '\n') {
			break;
		}
		seq[i] = seqString[i];
	}

	return atoi(seq);
}


static inline int getMsgType(char * seqString, size_t len, char * msgType) {
	
	size_t i;
	for (i = 0; i < len; i++) {
		if (seqString[i] == '\n') {
			break;
		}
		msgType[i] = seqString[i];
	}
}


static inline void getData(char * seqString, size_t len, char *data) {
	
	size_t i;

	for (i = 0; i < len; i++) {
		if (seqString[i] == '\n') {
			size_t count = 0;
			for (size_t j = i+1; j < len; j++) {
				data[count++] = seqString[j];
			}
			break;
		}
	}
}

static int reliableWRQ(int sock, char *filename, struct sockaddr_in servAddr, socklen_t servAddrLen) {
	
	int payloadSize = 0;

	char seqString[CHUNK+SEQ_SIZE+3];
	
	sprintf(seqString, "WRQ\n%s",filename);
	// WRQ
	payloadSize = sendto(sock, seqString, CHUNK+SEQ_SIZE+3, 0, (struct sockaddr *) &servAddr, servAddrLen);
	errorMsg(payloadSize, "ERROR for WRQ");
	
	printf("SEND WRQ : %s\n", filename);

	// WRQ ACK
	payloadSize = recvfrom(sock, seqString, SEQ_SIZE+3, 0, (struct sockaddr *) &servAddr, &servAddrLen);
	errorMsg(payloadSize, "ERROR for WRQ ACK");
	
	printf("WRQ ACK GET\n");
	return 0;
}
static int reliableSender(int sock, char *buffer, size_t buffer_len, int SEQ, struct sockaddr_in servAddr, socklen_t servAddrLen) {
	
	int payloadSize = 0;

	char seqString[CHUNK+SEQ_SIZE+3];
	memset(seqString, 0, CHUNK+SEQ_SIZE+3);

	// DATA payload  with SEQ
	sprintf(seqString, "DAT\n%d\n", SEQ);
	strcat(seqString, buffer);
	payloadSize = sendto(sock, seqString, CHUNK+SEQ_SIZE+3, 0, (struct sockaddr *) &servAddr, servAddrLen);
	errorMsg(payloadSize, "ERROR writing payload");
	
	// WAIT FOR ACK SEQ
	payloadSize = recvfrom(sock, seqString, SEQ_SIZE+3, 0, (struct sockaddr *) &servAddr, &servAddrLen);
	errorMsg(payloadSize, "ERROR recv payload");

	// check if wrong seq number
	if (getSeq(seqString, 20) != SEQ) {
		return 1;
	}

	return 0;
}

static int reliableReceiver(int sock, struct sockaddr_in clientAddr, socklen_t clientAddrLen, char *payload) {
	
	int payloadSize = 0;
	char buffer [CHUNK+SEQ_SIZE+3];
	char seqString [CHUNK+SEQ_SIZE];
	char data [CHUNK];
	char msgType[3];
	int seq = 0;
	int RET = 0;

	// receive packet
	payloadSize = recvfrom(sock, buffer, CHUNK+SEQ_SIZE+3, 0, (struct sockaddr *) &clientAddr, &clientAddrLen);
	errorMsg(payloadSize, "ERROR receive");
	
	getMsgType(buffer, CHUNK+SEQ_SIZE+3, msgType);
	getData(buffer, CHUNK+SEQ_SIZE+3, seqString);

	if (strcmp(msgType, "WRQ") == 0) {
		seq = 0;
		RET = 1;
		strcpy(payload, seqString);
	} else if (strcmp(msgType, "DAT") == 0) {
		getData(buffer, CHUNK+SEQ_SIZE+3, seqString);
		seq = getSeq(seqString, CHUNK+SEQ_SIZE);
		RET = 2;
		getData(seqString, CHUNK+SEQ_SIZE, data);
		strcpy(payload, data);
	}
	
	// return ACK
	char retACK[SEQ_SIZE+3];
	sprintf(retACK,"%d\nACK",seq);

	payloadSize = sendto(sock, retACK, SEQ_SIZE+3, 0, (struct sockaddr *) &clientAddr, clientAddrLen);
	errorMsg(payloadSize, "ERROR for ack seq");
	
	return RET;
}

#endif
