#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#define PORT "8888"

typedef struct frame {
	int seq;
	char buf[1024];
	int read_bytes;
}_frame_;

int main(int argc, char *argv[])
{

    if(argc < 4) {
        printf("args! server_ip_address client_ip_address multicast/unicast multicast_file_no \n");
        exit(-1);
    }

	if((strcmp(argv[3],"multicast") == 0) && (argc < 5)) {
		printf("multicast_file_no !\n");
		exit(-1);
	}

    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; // use tcp to build connection

    int status;
    if ((status = getaddrinfo(argv[1],PORT,&hints,&servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(-1);
    }

    int sockfd;
    if ((sockfd = socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol)) == -1) {
        perror("client: socket");
        exit(-1);
    }

    if (connect(sockfd,servinfo->ai_addr,servinfo->ai_addrlen) == -1) {
        close(sockfd);
        perror("client: socket");
    }

    printf("TCP connentction get !\n");

    close(sockfd);
    //freeaddrinfo(servinfo);
    //freeaddrinfo(&hints);
    // close tcp socket to do udp socket transfer

    //servinfo->ai_socktype = SOCK_DGRAM;
    struct addrinfo chints, *client;
    memset(&chints,'\0',sizeof(chints));
    memset(&client,'\0',sizeof(client));
    chints.ai_family = AF_INET;
    chints.ai_socktype = SOCK_DGRAM;
    chints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL,"7525",&chints,&client)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(-1);
    }

    int newsockfd;
    if ((newsockfd = socket(client->ai_family,client->ai_socktype,0)) == -1) {
        perror("client: socket");
        exit(-1);
    }
    int reuse = 1;
    setsockopt(newsockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse,sizeof(reuse));
	
	
	bind(newsockfd,client->ai_addr,client->ai_addrlen);
    
	/*
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(newsockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
    */
    
	int numbytes;
	_frame_ pack;
	memset(&pack,'\0',sizeof(pack));
	pack.seq = 0;

    struct sockaddr_in cli_addr;
    socklen_t len = sizeof cli_addr;
    struct ip_mreq group;

    memset(&group,'\0',sizeof(group));
    memset(&cli_addr,'\0',sizeof(struct sockaddr_in));

    group.imr_multiaddr.s_addr = inet_addr("226.1.1.1");
    group.imr_interface.s_addr = inet_addr(argv[2]);
	printf("ip address : %s \n",argv[2]);
    if (strcmp(argv[3],"multicast") == 0) {

        if(setsockopt(newsockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0) {
            perror("Adding multicast group error");
            exit(1);
        } else {
            printf("multicasting!\n");
        }
    } else if (strcmp(argv[3],"unicast") == 0) {
        printf("unicast!\n");
    } else {
        printf("args !\n");
        exit(1);
    }


    //char rec[20];
    //read(newsockfd,rec,20);

    //if(recvfrom(newsockfd,rec,sizeof(rec),0,(struct sockaddr *)&cli_addr,&len) < 0) {
    //	perror("recv failed");
    //	exit(1);
    //}
    //printf("%s\n",rec);
    //printf("recv success\n");
    // see if multicast or unicast

    char source[30] = "recvs/";
    unsigned long recv_size = 0;

    unsigned long file_len = 0;
    if ((numbytes = recvfrom(newsockfd,&file_len,sizeof(unsigned long),0,(struct sockaddr *)&cli_addr,&len)) == -1) {
        perror("recvfrom");
        exit(-1);
    }
	usleep(20000);

    /*
    int t_id = 0;
    if ((numbytes = recvfrom(newsockfd, &t_id,sizeof(unsigned long),0,(struct sockaddr *)&cli_addr,&len)) == -1) {
    	perror("recvfrom tid");
    	exit(-1);
    }

    */

    printf("The file size is : %lu\n",file_len);
    memset(pack.buf,'\0',sizeof(pack.buf));

    if ((numbytes = recvfrom(newsockfd,&pack,sizeof(pack),0,(struct sockaddr *)&cli_addr,&len)) == -1) {
		perror("recvfrom");
	}

	usleep(20000);

    //printf("The thread id is : %d\n", t_id);
    //printf("Start recv file : %s\n", pack.buf);
    char s[20];
	memset(s,'\0',20);
	if(pack.seq < 0) {
		int no = atoi(argv[4]);
		sprintf(s,"%d-",no);
	}
    else sprintf(s,"%d-",pack.seq);
	char r[pack.read_bytes];
	strcpy(r,pack.buf);
    strcat(s,r);
    strcat(source,s);
	printf("Start recv file : %s\n",source);

    int fd = open(source,O_CREAT|O_WRONLY|O_TRUNC,0600);

    int loss_num = 0;
	int loss_rate = 0;
	pack.seq = 0;
	pack.read_bytes = 0;
	int end_num = file_len / 1024 + 1;
	time_t start_t,end_t;
	double diff_t;

    while(pack.seq < end_num) {
        memset(pack.buf,'\0',sizeof(pack.buf));
		time(&start_t);
		
		if ((numbytes = recvfrom(newsockfd,&pack,sizeof(pack),0,(struct sockaddr *)&cli_addr,&len)) == -1) {
            perror("recvfrom");
        }
		
		time(&end_t);
		diff_t = difftime(end_t,start_t);
		
		// time out
		if(diff_t > 1) continue;
        
		printf("packet seq : %d\n", pack.seq);
		printf("packet bytes: %d\n", pack.read_bytes);

		if (pack.seq <= end_num ) write(fd,pack.buf,pack.read_bytes);
       	
		loss_num++;
		
		if(loss_num != pack.seq) {
			printf(" no. packet loss : %d \n", pack.seq );
			loss_rate++;
		}
		
		// if((pack.seq*100/end_num) % 5 == 0) printf("file transfer : %d \n",(pack.seq*100/end_num));
        // printf("%s\n",buf);
        
		recv_size += pack.read_bytes;
		usleep(50000);
    }
	printf("recv size : %lu\n",recv_size);
	printf("packet loss rate : %d \n", loss_rate*100/end_num);
    //printf("loss rate : %f\n",(float)loss_num*/file_len);
    close(fd);
    close(newsockfd);

    return 0;
}
