#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>



/*Structure for a datagram*/
typedef struct packet{
    int packetKind; // 0 -Ack, 1- Data
    int seqNumber; //
    int ack;
    char data[100];
}Packet;



void main(int argc, char *argv[]){
    char     *szAddress;             /*  Holds remote IP address   */
    char     *szPort;                /*  Holds remote port         */
    char     *filePath;              /*  Holds file path           */
    char     *toFormat;              /*  Holds target file format  */
    char     *toName;   
    char     *lossProbab;
    char     *randomSeed;

    if(argc != 7){
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
    int lossProbability = atoi(lossProbabChar);
    int randomSeed = atoi(randomSeedChar);

    int sockfd;
    struct sockaddr_in serverAddr;
    char buffer[1024];
    socklen_t addr_size;

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&serverAddr, '\0', sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");



    int fileLen;
    FILE *filePtr;
    filePtr = fopen(filePath, "rb");
    if (filePtr==NULL){
        printf("Error opening file!");
        exit(1);
    }
  
    //Get the length of file
    fseek(filePtr,0, SEEK_END); 
    fileLen= ftell(filePtr); 
    printf("the length of the entire file is: %d\n",fileLen);
    fseek(filePtr, 0, SEEK_SET);



    //make packets to send 
    int packetId = 0;
    Packet sendPacket;
    Packet recvPacket;
    int ackRcv = 1;



    while(1){

        if (ackRcv ==1){
            sendPacket.seqNumber = packetId;
            sendPacket.packetKind = 1; /* 0 -Ack, 1- Data */
            sendPacket.data = 

            lossy_sendto(sockfd, )
        }

    }



    //strcpy(buffer, "Hello Server\n");
    //sendto(sockfd, buffer, 1024, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    //printf("[+]Data Send: %s", buffer);

}

void makePacket(){

}