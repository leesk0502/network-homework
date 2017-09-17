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

#define BUFF_SIZE 2048

struct spacket
{
  int seq;
  size_t length;
  char data[BUFF_SIZE];
};

int main(int argc, char **argv){
  int sd,connfd,len;
  ssize_t recv_size;
  FILE *fp;
  size_t total_file_size;
  int i=0, last_i=0;

  struct spacket curpacket, ackpacket;
  struct sockaddr_in servaddr,cliaddr;

  if(argc!=2){
    printf("Usage : %s <PORT>\n", argv[0]);
    exit(1);
  }

  sd = socket(AF_INET, SOCK_DGRAM, 0);

  if(sd==-1){
    printf(" socket not created in server\n");
    exit(0);
  }
  else{
    printf("socket created in  server\n");
  }

  bzero(&servaddr, sizeof(servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(atoi(argv[1]));

  if ( bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0 )
    printf("Not binded\n");
  else
    printf("Binded\n");

  len=sizeof( cliaddr );

  while(1){
    // First data must be non block
    memset(&curpacket, 0, sizeof(curpacket));
    printf("Wait data\n");
    recv_size = recvfrom( sd, &curpacket, sizeof(curpacket), 0, (struct sockaddr *)&cliaddr, &len );

    curpacket.seq = ntohl(curpacket.seq);
    curpacket.length = ntohl(curpacket.length);

    // If process end but send last data, then just send ack packet
    if( i == 0 && curpacket.length <= 0 && curpacket.seq == last_i ){
      memset(&ackpacket, 0, sizeof(ackpacket));
      // Set next expected seq and send
      ackpacket.seq = htonl(last_i+1);
      sendto( sd, &ackpacket, sizeof(ackpacket), 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr) );
      continue;
    }

    printf("Get : seq(%d), data_size(%d)\n", curpacket.seq, curpacket.length);

    // If expected seq number
    if( curpacket.seq == i ){
      memset(&ackpacket, 0, sizeof(ackpacket));
      // Set next expected seq and send
      ackpacket.seq = htonl(i+1);
      sendto( sd, &ackpacket, sizeof(ackpacket), 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr) );
      printf("send : ack(%d)\n", ntohl(ackpacket.seq));

      // First packet is include File name
      if( i == 0 ){
        printf("Get File Name : %s\n", curpacket.data);
        fp=fopen( curpacket.data, "wb" );
        if( fp==NULL ){
          printf("Error\n");
          break;
        }
        // file open success last_i init
        last_i = 0;
      }
      // else fwrite data
      else {
        if( fp==NULL ){
          printf("No file pointer\n");
          continue;
        }
        if( curpacket.length > 0 ){
          fwrite( curpacket.data, sizeof(char), curpacket.length, fp);
        } else {
          // If get last packet, init
          last_i = i;
          i = -1;
          fclose(fp);
          printf("flocse\n");
        }
      }
      // Increase i
      i++;
    } else {
      // Resend ack packet
      sendto( sd, &ackpacket, sizeof(ackpacket), 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr) );
    }
  }

  close(sd);
  return(0);
}
