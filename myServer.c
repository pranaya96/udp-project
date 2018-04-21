#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>


#include "sendlib.c"
#include "helper.c"

#define PAYLOAD_SIZE 20		//size of data payload of the RDT layer
#define HEADER_SIZE 6	
#define TIMEOUT 50000		//50 milliseconds
#define TWAIT 10*TIMEOUT	//Each peer keeps an eye on the receiving  end for TWAIT time units before closing 
#define MAX_LINE 200                         //For retransmission of missing last ACK



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
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        perror("bind failed");
        return 0;
    }

    printf("Socket binded successfully\n");

    addr_size = sizeof(my_addr);

    char packet_identifier = 'n'; //for normal packet packet identifier is 'n'
    char curr_seq_no = '0';
    char recv_buffer[PAYLOAD_SIZE+HEADER_SIZE];
    char targetType;
    int targetFileNameLength;
    char *targetFileName;
    int data_size;
    int offset;
    unsigned char* data_recv = (unsigned char*) malloc(10);
    unsigned char* recv_ptr = data_recv;



    int r;
    char seq_num;

    // if (recvfrom(sockfd, recv_buffer, PAYLOAD_SIZE+HEADER_SIZE, 0, (struct sockaddr *)&my_addr, &addr_size) > 0) {
    //     printf("The value is %c\n", recv_buffer[0]);

    // }


    while (packet_identifier != 'l'){
        //printf("-----------------The program went here--------------\n");
        // if ((r = recvfrom(sockfd, recv_buffer, PAYLOAD_SIZE+HEADER_SIZE, 0, (struct sockaddr *)&my_addr, &addr_size) < 0)){
        //     printf("The value is %d\n", r);

        // }
        if ((r = recvfrom(sockfd, recv_buffer, PAYLOAD_SIZE+HEADER_SIZE, 0, (struct sockaddr *)&my_addr,&addr_size) > 0)) {
            printf("-----------------The program went here--------------\n");
             memcpy(&seq_num, recv_buffer, 1);
             printf("Packet with sequence number %c is received \n", seq_num);
             if (curr_seq_no == seq_num) {
                 memcpy(&packet_identifier, recv_buffer+1, 1);
                 printf("PacketIDentifier %c recieved\n", packet_identifier);
                 if (packet_identifier == 'h') {
                    recv_buffer[PAYLOAD_SIZE+HEADER_SIZE-1] = '\0';
                    printf("Buffer is received %s\n", recv_buffer);
                    memcpy(&targetType, recv_buffer+6, 1);
                    memcpy(&targetFileNameLength, recv_buffer+7, 4);
                    targetFileName = (char*) malloc (targetFileNameLength);
                    memcpy(targetFileName, recv_buffer+11, targetFileNameLength);
                    // for (int i=0; i<targetFileNameLength; i++) {
                    //     printf("%c\n", targetFileName[i]);
                    // }
                 } else {
                     memcpy(&data_size, recv_buffer+2, 4);
                     //printf("Data size: %d,  i: %d\n", payload_size, i);
                     data_recv = (unsigned char*) realloc(data_recv, offset+data_size);
                     recv_ptr = data_recv+offset;
                     memcpy(recv_ptr, recv_buffer+6, data_size);
                 }
                 printf("Val of data size: %d\n", data_size);
                 offset += data_size;
                 printf("changing seq no of data\n");
                 if (curr_seq_no == '0'){
                     curr_seq_no = '1';
                 }else{
                     curr_seq_no = '0';
                 }
        }
        printf("sending ack corrsponding to seq num\n");
        int m;
        m = lossy_sendto(lossProbab, randomSeed, sockfd, &seq_num, 1, (struct sockaddr *)&my_addr, sizeof(my_addr));
        if(m < 0) 
        {
            perror("Error sending ACK\n");
            exit(4);
        } 
        printf("ACK %c sent\n", seq_num);

    }
    
    printf("Total data size: %d\n", offset);

    }
   
    //check the recived contents by printing
    // for (int i=0; i<offset; i++) {
       //printf("%c\n", data_recv[i]);
    
    ///implement convert
    
    int written;
    written = offset-4;
    char tempBuffer[written];
    printf("Fine until memcpy\n");
    memcpy(tempBuffer,data_recv+4,written);
    printf("The total length written is%d\n", written);
    
    FILE *newFilePtr;
    newFilePtr = fopen(targetFileName, "wb");
    if (newFilePtr==NULL){
        printf("Error opening file!");
        exit(1);
    }
   
    int pointerPosition;
    pointerPosition = 0;
    //printf("Fine until while loop \n");
    while (pointerPosition < written-1){
        if (tempBuffer[pointerPosition] == 0) {
            pointerPosition = readTypeZero(tempBuffer, pointerPosition, newFilePtr, &targetType);
        }
        if (tempBuffer[pointerPosition]== 1){
            pointerPosition = readTypeOne(tempBuffer, pointerPosition, newFilePtr, &targetType);
        }
    }

    fclose(newFilePtr);


     //}

   

    //  if (close(sockfd) < 0 ){
    //      perror("Closing connection");
    //      exit(4);
    //     }

    return 0;
}



    


    

