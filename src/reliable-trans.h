#ifndef RELIABLE_TRANS_H
#define RELIABLE_TRANS_H

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


// Multiplex for getting seq numbers

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

// Multiplex for getting msg type.
// I.e. WRQ, DAT 

static inline int getMsgType(char * seqString, size_t len, char * msgType) {
	
	size_t i;
	for (i = 0; i < len; i++) {
		if (seqString[i] == '\n') {
			break;
		}
		msgType[i] = seqString[i];
	}
}

// Multiplex for playload

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

// Reliable WRQ message with ACK

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

// Reliable DATA+SEQ transfer with ACK

static int reliableSender(int sock, char *buffer, size_t buffer_len, int SEQ, struct sockaddr_in servAddr, socklen_t servAddrLen) {
	
	int payloadSize = 0;

	char seqString[CHUNK+SEQ_SIZE+3];
	memset(seqString, 0, CHUNK+SEQ_SIZE+3);
	
	// DEFINE RET
	// 0 : NORMAL
	// 1 : ERROR, RESEND

	// DATA payload  with SEQ
	if (buffer_len == CHUNK) {
		sprintf(seqString, "DAT\n%d\n", SEQ);
	} else {
		sprintf(seqString, "END\n%d\n", SEQ);
	}
	strcat(seqString, buffer);
	
	// send
	payloadSize = sendto(sock, seqString, CHUNK+SEQ_SIZE+3, 0, (struct sockaddr *) &servAddr, servAddrLen);
	errorMsg(payloadSize, "ERROR writing payload");
	
	// WAIT FOR ACK SEQ
	payloadSize = recvfrom(sock, seqString, SEQ_SIZE+3, 0, (struct sockaddr *) &servAddr, &servAddrLen);
	errorMsg(payloadSize, "ERROR recv payload");
	printf("RECEIVED ACK : %s\n", seqString);
	// check if wrong seq number
	if (getSeq(seqString, 20) != SEQ) {
		return 1;
	}

	return 0;
}

// Reliable receiver with ACK+SEQ response

static int reliableReceiver(int sock, struct sockaddr_in clientAddr, socklen_t clientAddrLen, char *payload) {
	
	int payloadSize = 0;
	char buffer [CHUNK+SEQ_SIZE+3];
	char seqString [CHUNK+SEQ_SIZE];
	char data [CHUNK];
	char msgType[3];
	int seq = 0;
	int RET = 0;

	// DEFINE RET NUMBER
	// 0 : ERROR/PACKET LOSS
	// 1 : WRQ
	// 2 : WRITE DATA
	// 3 : END
	
	// receive packet
	payloadSize = recvfrom(sock, buffer, CHUNK+SEQ_SIZE+3, 0, (struct sockaddr *) &clientAddr, &clientAddrLen);
	errorMsg(payloadSize, "ERROR receive");
	
	getMsgType(buffer, CHUNK+SEQ_SIZE+3, msgType);
	getData(buffer, CHUNK+SEQ_SIZE+3, seqString);

	// check msge type

	if (strcmp(msgType, "WRQ") == 0) {
		seq = 0;
		RET = 1;
		strcpy(payload, seqString);
	} else if (strcmp(msgType, "DAT") == 0) {
		// get seq
		getData(buffer, CHUNK+SEQ_SIZE+3, seqString);
		seq = getSeq(seqString, CHUNK+SEQ_SIZE);
		
		getData(seqString, CHUNK+SEQ_SIZE, data);
		strcpy(payload, data);
		
		RET = 2;

		// check chunk for packet loss
		if (strlen(payload) != CHUNK) {
			RET = 0;
			seq = -1;
		}

	} else if (strcmp(msgType, "END") == 0) {
		getData(buffer, CHUNK+SEQ_SIZE+3, seqString);
		seq = getSeq(seqString, CHUNK+SEQ_SIZE);
		getData(seqString, CHUNK+SEQ_SIZE, data);
		strcpy(payload, data);
		RET = 3;
	} else {
		// ERROR
		seq = -1;
		RET = 0;
	}
	
	// return ACK
	char retACK[SEQ_SIZE+3];
	sprintf(retACK,"%d\nACK",seq);

	payloadSize = sendto(sock, retACK, SEQ_SIZE+3, 0, (struct sockaddr *) &clientAddr, clientAddrLen);
	errorMsg(payloadSize, "ERROR for ack seq");
	printf("SEND ACK : %s\n", retACK);
	return RET;
}

#endif
