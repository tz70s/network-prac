#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern void tcpServerRun(char *);
extern void tcpClientRun(char *, char *, char *);
extern void udpServerRun(char *);
extern void udpClientRun(char *, char *, char *);

void printUsage() {
	// print the usage
	printf("Not enough arguments\n");
	printf("\nUSAGE\n\n");
	printf("./file-transfer [send/recv] [tcp/udp] [host] [port] [filename]\n\n");
	exit(0);
}

int main(int argc, char *argv[]) {
	
	// parse args
	if ( argc < 5 ) {
		printUsage();
	}

	char *role = argv[1];
	char *protocol = argv[2];
	char *host = argv[3];
	char *port = argv[4];
	char *fileName = argv[5];
	
	// Check roles
	if (strcmp(role,"recv") == 0) {

		// Check protocol
		if(strcmp(protocol, "tcp") == 0) {
			tcpServerRun(port);
		} else if (strcmp(protocol, "udp") == 0) {
			udpServerRun(port);
		} else {
			printUsage();
		}

	} else if (strcmp(role, "send") == 0) {
		
		if ( argc < 6 ) {
			printUsage();
		}
		if(strcmp(protocol, "tcp") == 0) {
			tcpClientRun(host, port, fileName);
		} else if (strcmp(protocol, "udp") == 0) {
			udpClientRun(host, port, fileName);
		} else {
			printUsage();
		}

	} else {
		printUsage();
	}

	return 0;
}
