#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <resolv.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define NAME_BUFF_SIZE 256
#define BUFF_SIZE 1024
#define TIMEOUT 400000

struct spacket
{
  int seq;
  size_t length;
  char data[BUFF_SIZE];
};

int main(int argc, char **argv){
  int sd,connfd,len;
  ssize_t total_file_size, file_size;
  FILE *fp;
  int i=0;

  struct spacket curpacket, ackpacket;
  struct sockaddr_in servaddr,cliaddr;
  struct timeval tv;

  if(argc!=4){
    printf("Usage : %s <IP> <PORT> <FILE_NAME>\n", argv[0]);
    exit(1);
  }

  // File name must less then BUFF_SIZE / 2
  if(strlen(argv[3]) > NAME_BUFF_SIZE / 2){
    printf("File name Size cannot exceed %d", BUFF_SIZE / 2);
    exit(1);
  }

  // create socket in client side
  sd = socket( PF_INET, SOCK_DGRAM, 0 );

  if(sd==-1){
    printf(" socket not created in client\n");
    exit(0);
  }
  else{
    printf("socket created in  client\n");
  }


  memset( &servaddr, 0, sizeof(servaddr) );

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr( argv[1] );
  servaddr.sin_port = htons( atoi(argv[2]) );

  // Open File
  fp=fopen(argv[3],"rb");
  if( fp == NULL ){
    printf("File does not exist\n");
    exit(1);
  }

  // Set timeout
  tv.tv_sec = 0;
  tv.tv_usec = TIMEOUT;
  if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
    perror("Error");
  }

  // Send data
  while(1){
    // Reset pakcet buffer
    memset(&curpacket, 0, sizeof(curpacket));

    // First Send file name to Server
    if( i == 0 ){
      curpacket.seq = htonl(0);
      curpacket.length = htonl(strlen(argv[3]));
      strcpy(curpacket.data, argv[3]);
    } else {
      memset(curpacket.data, 0, sizeof(curpacket.data));
      file_size = fread(curpacket.data, sizeof(char), sizeof(curpacket.data), fp);
      // Start at seq num 0
      curpacket.seq = htonl(i);
      curpacket.length = htonl(file_size);
    }

    sendto( sd, &curpacket, sizeof(curpacket), 0, (struct sockaddr *)&servaddr, sizeof(struct sockaddr) );
    printf("send : seq(%d), file_size(%d)\n", i, file_size);

    // Wait Ack Packet
    while(1){
      memset(&ackpacket, 0, sizeof(ackpacket));
      if( recvfrom( sd, &ackpacket, sizeof(ackpacket), 0, (struct sockaddr*)&servaddr, &len) < 0 ){
        // Timeout Retransmit data
        sendto( sd, &curpacket, sizeof(curpacket), 0, (struct sockaddr *)&servaddr, sizeof(struct sockaddr) );
        printf("resend : seq(%d), file_size(%d)\n", i, file_size);
        continue;
      }
      ackpacket.seq = ntohl(ackpacket.seq);
      printf("respond: ack(%d)\n", ackpacket.seq);

      // If expected ack same with next seq number, break
      if( (i+1) == ackpacket.seq ){
        break;
      }
    }

    // End of file
    if( i != 0 && file_size <= 0 ){
      break;
    }

    // Increase i
    i++;
  }



  //close client side connection
  fclose(fp);
  close(sd);

  return(0);
}
