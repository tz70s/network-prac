#ifndef INITSERVER_H
#define INITSERVER_H

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

static void errorMsg(int descriptor, char * msg) {
	if (descriptor < 0) {
		perror(msg);
		exit(1);
	}
}

static struct sockaddr_in initAddr(char *role, char *host, int port) {
	
	// create sock addr for ip address and port
	struct sockaddr_in servAddr;
	
	// intialized
	memset(&servAddr, 0, sizeof(servAddr));

	// set sock addr
	servAddr.sin_family = AF_INET;

	// set the address
	if (strcmp(role, "recv") == 0) {
		servAddr.sin_addr.s_addr = INADDR_ANY;
	} else if (strcmp(role, "send") == 0) {
		servAddr.sin_addr.s_addr = inet_addr(host);
	} else {
		perror("ERROR creating addr object");
		exit(1);
	}

	servAddr.sin_port = htons(port);
	return servAddr;
}


static int initTCPServer(int port) {
	
	// create sockfile
	int servSock = socket(AF_INET, SOCK_STREAM, 0);

	// create sock addr for ip address and port
	struct sockaddr_in servAddr = initAddr("recv", NULL, port);

	// bind
	bind(servSock, (struct sockaddr*)&servAddr, sizeof(servAddr));
	
	return servSock;
}

static int initUDPServer(int port) {
	
	// create sockfile
	int servSock = socket(AF_INET, SOCK_DGRAM, 0);

	// create sock addr for ip address and port
	struct sockaddr_in servAddr = initAddr("recv", NULL, port);

	// bind
	bind(servSock, (struct sockaddr*)&servAddr, sizeof(servAddr));
	
	return servSock;

}

static int initTCPClient(char *host, int port) {
	
	// create sock
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	// server addr
	struct sockaddr_in servAddr = initAddr("send", host, port);

	// connect
	connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr));
	return sock;
}

static int initUDPClient() {
	
	// create sock
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	// udp connectionless
	
	return sock;
}

static struct sockaddr_in initUDPAddr(char *host, int port) {

	// server addr
	struct sockaddr_in servAddr = initAddr("send", host, port);

	return servAddr;
}
#endif
