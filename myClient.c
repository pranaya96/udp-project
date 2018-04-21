#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>


#include "sendlib.c"

#define PAYLOAD_SIZE 20		//size of data payload of the RDT layer
#define HEADER_SIZE 6	
#define TIMEOUT 50000		//50 milliseconds
#define TWAIT 10*TIMEOUT	//Each peer keeps an eye on the receiving  end for TWAIT time units before closing 
#define MAX_LINE 200                         //For retransmission of missing last ACK

char *make_packet(char* , char* , int* , char*);

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

    int port = atoi(argv[2]);
    double lossProbab = atof(lossProbabChar);
    int randomSeed = atoi(randomSeedChar);

    int sockfd;
    struct sockaddr_in serverAddr;
    char seq_num;
    int packet_num;
    char curr_ack;
    char *data;
    int data_size;
    char packet_identifier;


    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket");
        return 0;
    }

    memset(&serverAddr, '\0', sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    //serverAddr.sin_addr.s_addr = htonl(szAddress);    
    serverAddr.sin_port = htons(port);

    if (inet_aton(szAddress, &serverAddr.sin_addr) <= 0 ) {
        perror("Error Setting server IP address");
    }

//    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
//          perror("bind failed");
//        return 0;
//     }

    int fileLen, len;
    //nt sent=0;
    char msg[PAYLOAD_SIZE];
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
    
    int number_of_packets = (int) ceil((1.0*fileLen) / PAYLOAD_SIZE);
    printf("The toatl number of packets: %d\n", number_of_packets);
    seq_num = '0' ;  // either '1' or '0'
    packet_num = -1 ; // increase till the last packet from -1 i.e -1 being header packet
    
    char fileBuffer[fileLen];
    
    fread(fileBuffer,1,fileLen,filePtr);
    struct timeval timer;
    
    // if (sendto(sockfd, fileBuffer, fileLen, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    //     printf("Error sending\n");
    // }


    fd_set read_fds;//read file descriptor set
    FD_ZERO(&read_fds);

    FD_SET(sockfd, &read_fds);

    while (packet_num < number_of_packets) { //there are n+1 packets first one being the packet with all those header
        // in case of the last packet vs other packets
        if (packet_num == number_of_packets-1) { 
            data_size = fileLen - (packet_num)*PAYLOAD_SIZE;
            packet_identifier = 'l'; //packet identifer: 'l' -> last packet, 'n' -> normal packet, 'h' -> header packet
        }
        else {
            data_size = PAYLOAD_SIZE;
            packet_identifier = 'n';
        }
        
        //in case of header packet(1st packet) vs other packets
        if (packet_num == -1) {
            int targetFileNameLength = strlen(toName); 
            data_size = targetFileNameLength+5;
            data = (char*) malloc(data_size);
            packet_identifier = 'h';
            memcpy(data, toFormat, 1);
            memcpy(data+1, &targetFileNameLength, 4);
            memcpy(data+5, toName, targetFileNameLength);
        }

        else {
            data = (char*) malloc(data_size);
            memcpy(data, fileBuffer+packet_num*PAYLOAD_SIZE, data_size);
        }
        
        len = data_size+HEADER_SIZE;
        char *send_pack = make_packet(&packet_identifier, &seq_num, &data_size, data);
        printf("Sending Packet with sequence number  %c\n", seq_num);

        int test;
        test = lossy_sendto(lossProbab, randomSeed, sockfd, send_pack, len, (struct sockaddr *)&serverAddr, sizeof(serverAddr)); 
        //test = sendto(sockfd, send_pack, len, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)); 
        printf("WERRRROOROROROROROORRORORO: %d\n", test);
        if (test < 0) {
            perror("Error sending the the packet\n");
            exit(4);
        }
        /* TIMER COMES INTO PLAY */

        timer.tv_sec = 0;
        timer.tv_usec = TIMEOUT;

        int status;
        status = select(sockfd+1, &read_fds, NULL, NULL, &timer);
        if (status == -1){
            perror("select");
            exit(4);
        } else if(status == 0){
            printf("Timeout occured ! Retrasmitting the packet...\n");
            FD_SET(sockfd, &read_fds);
            // dynamically deallocating the data and packet to be sent
            free(data);
            free(send_pack);
            continue;
        }

        socklen_t addr_size = sizeof(serverAddr);
         /*  RECIEVING THE ACK */
         int n = 0;
         if ((n = recvfrom(sockfd, &curr_ack, 1, 0, (struct sockaddr *)&serverAddr, &addr_size)) < 0) 
         {
            perror("error receiving ACK");
            exit(4);
         } 
 
         if (curr_ack == seq_num) {
             printf("ACK%c received \n", curr_ack);
             if (seq_num == '0'){
                 seq_num = '1';
             }else{
                 seq_num = '0';
             }
        packet_num++;
        }   
         // dynamically deallocating the data and packet to be sent
         free(data);
         free(send_pack);
         
     }
     fclose(filePtr);
     return 0;


}

char *make_packet(char* pkt_id, char* pkt_seq, int* size_data, char* data) //every packet has a 6-byte header
{
    char* packet = (char*) malloc(PAYLOAD_SIZE+HEADER_SIZE);
    memcpy(packet, pkt_seq, 1);
    memcpy(packet+1, pkt_id, 1);
    memcpy(packet+2, size_data, 4);
    memcpy(packet+6, data, *size_data);
    return packet; 
}

