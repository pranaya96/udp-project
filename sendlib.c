#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>


typedef unsigned char		u8b_t;    	// a char
typedef unsigned short	u16b_t;  	// 16-bit word
typedef unsigned int		u32b_t;		// 32-bit word

#define PAYLOAD 20		//size of data payload of the RDT layer
#define TIMEOUT 50000		//50 milliseconds
#define TWAIT 10*TIMEOUT	//Each peer keeps an eye on the receiving  end for TWAIT time units before closing 
                          //For retransmission of missing last ACK


                          
// define your data structures and global variables in here
u8b_t current_ACK=0;  //the same as expected_seqnum in FSM
u8b_t current_pkt=0;  //the same as next_seqnum in FSM
/* 
 * This function must NOT be modified.
 */
ssize_t lossy_sendto (float loss_ratio, unsigned int random_seed, int sockfd, const void * buf, size_t len,
                     const struct sockaddr_in *dest_addr, socklen_t addrlen)
{
  int n;
  float f;
  static int init = 0;
  
  if (! init) {
    srand (random_seed);
    init = 1;
  }
  
  f = ((float) rand ()) / RAND_MAX;
  
  /* Simulate segment loss */
  if (f < loss_ratio) return len;
  
  return sendto(sockfd, buf, len, 0, dest_addr, addrlen);  
}



char* make_packet(char* msg, int length, int seq) //every packet has a 2-byte header
{
    u8b_t* packet=(u8b_t*)malloc(length+2);
    packet[0]=1;   //1st byte: packet type: 1-data, 2-ACK

    packet[1]= seq;  //initial empty seq# field, 1-byte in total
    memcpy(packet+1,&seq,1);

    memcpy(packet+2,msg,length); //data payload, maximum 20 bytes
    
//    printf("This packet is:");print_packet((char*)packet,length+3);printf("\n");  
    return (char*)packet;
    
}

char* make_ACK(int seq) //every ACK is 2-byte
{
    u8b_t* packet=(u8b_t*)malloc(2);
    
    packet[0]=2;   //1st byte: packet type: 1-data, 2-ACK
  
    packet[1]=seq;  //initial empty seq# field
    memcpy(packet+1,&seq,1);

    return (char*)packet;
}

int is_ACK(char* packet, int bytes){
  return (packet[0]==2); //check the packet type field
}

int get_seq(char* packet, int bytes){
  return packet[1]; //get the packet's seq #
}


void print_packet(char* packet, int length){  //print the packet content, e.g. for debugging convenience
  for (int i=0;i<2;i++){
      printf("%x ",packet[i]);
  }
}

/* Application process calls this function to transmit a message to
   target remote process through RDT socket; this call will
   not return until the whole message has been successfully transmitted
   or when encountered errors.
*/
int rdt_send(int fd, char * msg, int length, double lossProbab, int randomSeed,  struct sockaddr_in serverAddr){
  //implement the Stop-and-Wait ARQ (rdt3.0) logic
  
  int pktLen= 2+length;
  char* packet=make_packet(msg,length,current_pkt); //make packet with the current packet sequence number
  
  lossy_sendto(lossProbab, randomSeed, fd ,packet,pktLen,&serverAddr, sizeof(serverAddr));
  printf("rdt_send: Sent one message of size %d\n",pktLen);
  struct timeval timer;
  int status;
  
  fd_set read_fds; //read file descriptor set


  for (;;){
    FD_ZERO(&read_fds);
    FD_SET(fd,&read_fds);

    timer.tv_sec=0;
    timer.tv_usec=TIMEOUT;  //set timeout interval
    if ((status=select(fd+1,&read_fds,NULL,NULL,&timer))==-1){  //select() failed
        perror("select");
        exit(4);
    }
    else if (status==0){  //timeout occured
        printf("rdt_send: Timeout!! Retransmit the packet (seq# %d) again\n",current_pkt);
        //udt_send(fd,packet,pktLen,0);   
        lossy_sendto(lossProbab, randomSeed, fd ,packet,pktLen,&serverAddr, sizeof(serverAddr));//retransmit the packet
        continue; //restart select() & timeout timer
    }

    if (FD_ISSET(fd,&read_fds)){ //packet arrival
      char rcv_packet[PAYLOAD+4];
      int rcv_bytes;
      if ((rcv_bytes = recvfrom(fd, rcv_packet, PAYLOAD+2,0, &serverAddr, sizeof(serverAddr))) <= 0)
          // got error or connection closed by client
              perror("rdt_recv");
      else if (is_ACK(rcv_packet,rcv_bytes)){
          if (get_seq(rcv_packet,rcv_bytes)!=current_pkt){ //ignore unexpected packet 
            //printf("unexpected ACK\n");
            continue;
          }
          else{
            current_pkt=abs(current_pkt-1);  //accept expected ACK, change current_pkt_#
            printf("rdt_send: Received the ACK\n");
            return 0;
          }
      }

  }
  
  }
  return 0;
} 

/* Application process calls this function to wait for a message 
   from the remote process; the caller will be blocked waiting for
   the arrival of the message.
*/
int rdt_recv(int fd, char * msg, int length,struct sockaddr_in serverAddr, socklen_t slen, double lossProbab, int randomSeed){
  //implement the Stop-and-Wait ARQ (rdt3.0) logic
  char rcv_packet[PAYLOAD+2];
  int rcv_bytes;
  //printf("expecting pkt# %d\n",current_ACK);
  while ((rcv_bytes=recvfrom(fd, rcv_packet, PAYLOAD+2,0, &serverAddr, slen))>0){ //looping until recv expected ACK
    if (get_seq(rcv_packet, rcv_bytes)==current_ACK){ //if is expected packet
      printf("rdt_recv: Got an expected packet\n");
      char* ACK=make_ACK(current_ACK);
      lossy_sendto(lossProbab, randomSeed,fd,ACK,2,&serverAddr, slen);
      memcpy(msg,rcv_packet+2,rcv_bytes-2); //pass data payload to upper layer
      current_ACK=abs(current_ACK-1);
      return rcv_bytes-2;
    }else{
      if (get_seq(rcv_packet, rcv_bytes)!=current_ACK) 
      printf("received a retransmission DATA packet from peer!!\n");
      printf("rdt_recv: Retransmit the ACK packet\n");
      char* ACK=make_ACK(abs(current_ACK-1));
      lossy_sendto(lossProbab, randomSeed,fd,ACK,2,&serverAddr, slen);
    }
  }
  return -1;
}


/* Application process calls this function to close the RDT socket.
*/
int rdt_close(int fd, double lossProbab, int randomSeed,  struct sockaddr_in serverAddr){
        //implement the Stop-and-Wait ARQ (rdt3.0) logic
        struct timeval timer;
        int status;
        
        fd_set read_fds; //read file descriptor set
        for (;;){
          FD_ZERO(&read_fds);
          FD_SET(fd,&read_fds);
          timer.tv_sec=0;
          timer.tv_usec=TWAIT;
          if ((status=select(fd+1,&read_fds,NULL,NULL,&timer))==-1){  //select() failed
            perror("select for rdt_close()");
            exit(4);
          }
          else if (status==0){  //timeout occured, no ACK received
            printf("rdt_close: Nothing happened for 500.00 milliseconds\n");
            printf("rdt_close: Release the socket\n");
            return close(fd);
          }
          if (FD_ISSET(fd,&read_fds)){ //packet arrival
            char rcv_packet[PAYLOAD+2];
            int rcv_bytes;
            if ((rcv_bytes = recvfrom(fd, rcv_packet, PAYLOAD+2,0,&serverAddr, sizeof(serverAddr))) <= 0)
            perror("got error or connection closed by the other side");
            else if (get_seq(rcv_packet, rcv_bytes)<current_ACK){ //received previous DATA packet, re-enter TWAIT state
              char* ACK=make_ACK(current_ACK-1); //return ACK# with (expected_seqnum-1), i.e. the last sent ACK#
              lossy_sendto(lossProbab, randomSeed,fd,ACK,2,&serverAddr, sizeof(serverAddr));
              continue;
            }
            else if (get_seq(rcv_packet, rcv_bytes)>=current_ACK){ //ignore other non-retransmitted packet
              continue;
            }
          }
}
    return 0;
}



  
