#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "sendlib.c"





int main(int argc, char *argv[]){
    char     *szAddress;             /*  Holds remote IP address   */
    char     *szPort;                /*  Holds remote port         */
    char     *filePath;              /*  Holds file path           */
    char     *toFormat;              /*  Holds target file format  */
    char     *toName;   
    char     *lossProbabChar;
    char     *randomSeedChar;

    if(argc != 8){
        printf("\nCommand Line Arguments not complete\n");
        exit(EXIT_FAILURE);
        return -1;
    }

    szAddress = argv[1] ;
    szPort = argv[2] ;
    filePath = argv[3] ;
    toFormat = argv[4];
    toName = argv[5];
    lossProbabChar = argv[6];
    randomSeedChar = argv[7];

    int port = atoi(argv[1]);
    double lossProbab = atof(lossProbabChar);
    int randomSeed = atoi(randomSeedChar);

    int sockfd;
    struct sockaddr_in serverAddr;


    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket");
        return 0;
    }

    memset(&serverAddr, '\0', sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(szAddress);    
    serverAddr.sin_port = htons(port);

    // if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    //     perror("bind failed");
    //     return 0;
    // }


    int fileLen, len;
    int sent=0;
    char msg[PAYLOAD];
    FILE *filePtr;
    filePtr = fopen(filePath, "rb");
    if (filePtr==NULL){
        printf("Error opening file!");
        exit(1);
        goto END;
    }
  
    //Get the length of file
    fseek(filePtr,0, SEEK_END); 
    fileLen= ftell(filePtr); 
    printf("the length of the entire file is: %d\n",fileLen);
    fseek(filePtr, 0, SEEK_SET);


    printf("checking for the segfault.......");

    memset(msg, '\0', PAYLOAD);
    sprintf(msg, "%d", fileLen);
    rdt_send(sockfd, msg, sizeof(msg), lossProbab, randomSeed, serverAddr);
    //sendto(sockfd, msg, sizeof(msg), 0,(const struct sockaddr *)&serverAddr, sizeof(serverAddr));  

    printf("Checking for the timeout.........");
    memset(msg, '\0', PAYLOAD);
    while (sent < fileLen) {
    	if ((fileLen-sent) < PAYLOAD)
    		len = fread(msg, sizeof(char), fileLen-sent, filePtr);
    	else
            len = fread(msg, sizeof(char), PAYLOAD, filePtr);
            printf("The length of 1st client packet sent is %d\n", len);
    	rdt_send(sockfd, msg, len, lossProbab, randomSeed, serverAddr);
    	sent += len;
    }
    
END:    
    // close the file
    fclose(filePtr);
    // close the rdt socket
    rdt_close(sockfd,lossProbab, randomSeed, serverAddr);
	printf("Client program terminated\n");
    return 0;

}

