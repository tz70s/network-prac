/*
    C socket server example, handles multiple clients using threads
    Compile
    gcc server.c -lpthread -o server
*/

#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>

//the thread function
void *connection_handler(void *);
void trans_handler(int,struct sockaddr_in,int,int);
struct in_addr localInterface;
struct sockaddr_in groupsock;

int thread_num;
char filename[20];
char type[10];
int wait_sig = 0;

typedef struct frame {
	int seq;
	char buf[1024];
	int read_bytes;
}_frame_;

int main(int argc , char *argv[])
{
    thread_num = -1;
    if(argc < 3) {
        perror("args! file multicast/unicast");
        exit(-1);
    }
    strcpy(filename,argv[1]);
    strcpy(type,argv[2]);
    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure

    memset(&server,'\0',sizeof(struct sockaddr_in));
    memset(&client,'\0',sizeof(struct sockaddr_in));
    memset((char *)&groupsock,0,sizeof(groupsock));

    groupsock.sin_family = AF_INET;
    groupsock.sin_addr.s_addr = inet_addr("226.1.1.1");
    groupsock.sin_port = htons(7525);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    pthread_t thread_id;

    while((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c))) {
        puts("Connection accepted");

        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client) < 0) {
            perror("could not create thread");
            return 1;
        }
        printf("%lu\n",thread_id);
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }

    if (client_sock < 0) {
        perror("accept failed");
        return 1;
    }

    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void * client)
{
    //Get the sock kint sock = *(int*)socket_desc;
    //char *message;
    struct sockaddr_in cli = *(struct sockaddr_in *) client;
    printf("hello world\n");
    char ipstr[20];
    void *addr;
    addr = &(cli.sin_addr);
 	thread_num++;   
	int t_id = thread_num;
	printf("the thread id is : %d \n",t_id);
    inet_ntop(cli.sin_family,addr,ipstr,sizeof(ipstr));
    printf("The ip accect : %s\n",ipstr);
    localInterface.s_addr = inet_addr(ipstr);
    //Send some messages to the client
    //message = "Greetings! Connetction get ! \n";
    //write(sock , message , strlen(message));
    int sockfd;

    if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) == -1) {
        perror("socket create failed");
        exit(-1);
    }

    while(thread_num < 2);

    if(strcmp(type,"multicast") == 0) {
       	if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface))) {
           	perror("Setting local interface error");
            exit(1);
        } else {
           	printf("Setting the local interface ... OK\n");
        //char st[] = "hello client";
        	printf("multicast !\n");
        //if(t_id < 20) {
        //	if(sendto(sockfd,st,strlen(st),0,(struct sockaddr *)&groupsock,sizeof(groupsock)) < 0) {
        //		perror("wrong of multicasting");
        //		exit(-1);
        //	} else {
        //		printf("send success\n");
        //	}
        //}
        //
        	trans_handler(sockfd,groupsock,1,t_id);
		}
	} else if(strcmp(type,"unicast") == 0) {
		while(wait_sig != t_id);
		printf("unicast !\n");
		cli.sin_port = htons(atoi("7525"));
		trans_handler(sockfd,cli,0,t_id);
		sleep(3);
		wait_sig++;

    } else {
        printf("wrong args : type\n");
		exit(-1);
    }

    close(sockfd);

    return 0;
}

void trans_handler(int sockfd,struct sockaddr_in cli, int flag, int t_id)
{

    FILE *fp = fopen(filename,"r");
    if(fp == NULL) {
        perror("cant open the file");
        exit(1);
    }

    unsigned long file_len;
    fseek(fp,0,SEEK_END);
    file_len = ftell(fp);
    fseek(fp,0,SEEK_SET);
    fclose(fp);

    int fd = open(filename,O_RDONLY);
    unsigned long sent_len = 0;

    printf("The file len is : %lu\n",file_len);
	
	int numbytes;

	if(((flag==1)&&(t_id==0))||(flag==0)) {
    	if((numbytes = sendto(sockfd,&file_len,sizeof(unsigned long),0,(struct sockaddr *)&cli,sizeof(cli))) == -1) {
        	perror("sendto failed");
        	exit(-1);
    	}
	}
	usleep(20000);

	printf("Start trans file : %s\n",filename);
	_frame_ pack;
	
	memset(&pack,'\0',sizeof(pack));
	pack.seq = t_id;
	if(flag==1) pack.seq = -1;
	strcpy(pack.buf,filename);
	pack.read_bytes = strlen(filename);

	if(((flag==1)&&(t_id==0))||(flag==0)) {
		
    	if((numbytes = sendto(sockfd,&pack,sizeof(pack),0,(struct sockaddr *)&cli,sizeof(cli))) == -1) {
        	perror("sendto failed");
        	exit(-1);
    	}
	}
	usleep(20000);

	memset(&pack,'\0',sizeof(pack));
	pack.seq = 0;
	pack.read_bytes = 0;

    while(sent_len < file_len) {
        memset(pack.buf,'\0',sizeof(pack.buf));
        int readsize = read(fd,pack.buf,1024);
		pack.seq++;
		pack.read_bytes = readsize;
		if(((pack.seq % 3) == t_id) || flag == 0) {
        	if((numbytes = sendto(sockfd,&pack,sizeof(pack),0,(struct sockaddr *)&cli,sizeof(cli))) == -1) {
            	perror("sendto failed");
            	exit(-1);
        	}
			printf("packet trans no.%d\n",pack.seq);
		}
		sent_len += readsize;
		usleep(50000);
		thread_num++;
   }
	/* end check */
	/*
	if((flag==0)||((flag==1)&&t_id==0)) {
		memset(pack.buf,'\0',sizeof(pack.buf));
		pack.seq++;
		sendto(sockfd,&pack,sizeof(pack),0,(struct sockaddr *)&cli,sizeof(cli));
		memset(pack.buf,'\0',sizeof(pack.buf));
		pack.seq++;
		sendto(sockfd,&pack,sizeof(pack),0,(struct sockaddr *)&cli,sizeof(cli));
	}
	*/

	close(sockfd);
    close(fd);
}
