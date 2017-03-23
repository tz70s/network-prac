#include "../reliable-trans.h"
#include "../init.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

int main() {
	
	printf("Test strip\n");
	char *samepleSeq = "235\nTHIS IS THE PAYLOAD\nYA";

	char data[1024];

	int seq = getSeq(samepleSeq, strlen(samepleSeq));
	
	printf("SEQ : %d\n", seq);
	
	getData(samepleSeq, strlen(samepleSeq), data);
	
	printf("DATA : %s\n", data);

	return 0;
}
