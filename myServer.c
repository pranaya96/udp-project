#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


/*Structure for a datagram*/
typedef struct datagram{
    int datagramKind;
    int seqNumber;
    int ack;
    char data[100];
}Datagram;



void main(int argc, char **argv){
    /* Get command line arguements */
    if(argc!=4){
        printf("\nCommand Line Arguments not complete\n");
        exit(EXIT_FAILURE);
        return -1;
    }

    int port = atoi(argv[1]);
    float  lossProbab = atoi(argv[2]);
    int randomSeed = atoi(argv[3]);

    /* scoket file descriptor*/
    int sockfd;

    struct sockaddr_in si_me, si_other;

    char buffer[1024];
    socklen_t addr_size;
  
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  
    memset(&si_me, '\0', sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(port);
    si_me.sin_addr.s_addr = inet_addr("127.0.0.1");
  
    bind(sockfd, (struct sockaddr*)&si_me, sizeof(si_me));
    addr_size = sizeof(si_other);
    recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*)& si_other, &addr_size);
    printf("[+]Data Received: %s", buffer);

  
  }



    

