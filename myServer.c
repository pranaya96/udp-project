#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "sendlib.c"


int main(int argc, char  *argv[]){
    char     *szPort;
    char     *lossProbabChar;
    char     *randomSeedChar;      
   
    /* Get command line arguements */
    if(argc!=4){
        printf("\nCommand Line Arguments not complete\n");
        exit(EXIT_FAILURE);
        return -1;
    }

    printf("2sHey this is server\n");

    szPort = argv[1];
    lossProbabChar = argv[2];
    randomSeedChar = argv[3];
    
    int port = atoi(szPort);
    float  lossProbab = atof(lossProbabChar);
    int randomSeed = atoi(randomSeedChar);

    /* scoket file descriptor*/
    int sockfd;
    struct sockaddr_in my_addr;
    socklen_t addr_size;
    
    
   
   
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket");
        return 0;
    }

    printf("Socket created successfully\n");

    memset(&my_addr, '\0', sizeof(my_addr));

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    //my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        perror("bind failed");
        return 0;
    }

    printf("Socket binded successfully\n");


    

    addr_size = sizeof(my_addr);
    


    // FILE *filePtr;
    // filePtr = fopen(filePath, "rb");
    // if (filePtr==NULL){
    //     printf("Error opening file!");
    //     exit(1);
    // }
    while(1){
    
    int fileLen, len;
    char msg[PAYLOAD];
    int received = 0;

    /* a very simple handshaking protocol */
	// wait for client request
	memset(msg, '\0', PAYLOAD);
    //dt_recv(int fd, char * msg, int length,struct sockaddr_in serverAddr, socklen_t slen)
    printf("Starting to recieve maal-----\n");
    len = rdt_recv(sockfd,msg, PAYLOAD, my_addr, addr_size, lossProbab, randomSeed);
   
    printf("recieved----yyaay-----\n");

	fileLen = atoi(msg);
	printf("Received client request: file size = %d\n", fileLen);

	// if (!testfile) {
	// 	printf("Cannot open the target file: ./%s for write\n", filepath);
		
	// 	// send the ERROR response
	// 	memset(msg, '\0', MSG_LEN);
	// 	sprintf(msg, "ERROR");
	// 	rdt_send(sockfd, msg, strlen(msg));
	// 	goto END;
	// } else {
	// 	printf("Open file %s for writing successfully\n", filepath);
	// 	// send the ERROR response
	// 	memset(msg, '\0', MSG_LEN);
	// 	sprintf(msg, "OKAY");
	// 	rdt_send(sockfd, msg, strlen(msg));
	// }

	/* start the file transfer */   
	printf("Start receiving the file . . .\n");
    // receive the file contents
    
    //while (received < fileLen) {
    	memset(msg, 0, PAYLOAD);
    	len = rdt_recv(sockfd, msg,PAYLOAD,my_addr, addr_size, lossProbab, randomSeed);
    	//fwrite(msg, sizeof(char), len, testfile);
    	received += len;
    	printf("Received a message of size %d bytes\n", len);
   // }

    

    printf("Complete the file transfer.\n");
    
END:
     // close the rdt socket
     rdt_close(sockfd, lossProbab, randomSeed, my_addr);
     
     printf("Server program terminated\n");
     return 0;
    }
}



    

