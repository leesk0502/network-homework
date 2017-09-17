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

#define PRESET_SIZE 5
#define BUFF_SIZE 2048
#define TIMEOUT 3

int main(int argc, char **argv){
  static const char PRESET_NAME[] = "FILE:";
  static const char PRESET_DATA[] = "DATA:";

  char temp_name[BUFF_SIZE]="";
  char buffer[BUFF_SIZE]="";
  int sd,connfd,len, file_recv_size=0;
  FILE *fp;
  size_t file_size; 
  struct sockaddr_in servaddr,cliaddr;
  int i=0, timestamp_sec=0;

  // Timeout value setting
  struct timeval tv;

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
    memset(buffer, 0, sizeof(buffer));
    file_recv_size = recvfrom( sd, buffer, BUFF_SIZE, 0, (struct sockaddr *)&cliaddr, &len );

    // Open file mode write
    timestamp_sec = (int)time(NULL);
    sprintf(temp_name, "%d", timestamp_sec);
    fp=fopen( temp_name, "wb" );
    if( fp==NULL ){
      printf("Error\n");
    }

    printf("Get First data\n");
    // Set timeout
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
      perror("Error");
    }
    
    while(1){
      if( strncmp(buffer, PRESET_NAME, PRESET_SIZE) == 0 ){
        // Rename File name and Break cause it is last data 
        memmove(&buffer[0], &buffer[PRESET_SIZE], strlen(buffer));
        printf("\n\nFile name : %s\n\n",buffer);
        if( rename( temp_name, buffer ) == -1 ){
          printf("Error file rename\n");
        }
        printf("File name received\n");
      } else {
        memmove(&buffer[0], &buffer[sizeof(PRESET_DATA)-1], sizeof(buffer));
        // wirte file
        fwrite( buffer, sizeof(char), file_recv_size-strlen(PRESET_DATA), fp);
      }

      i++;
      // If Client file end but last send data is loss, then timeout and break;
      memset(buffer, 0, sizeof(buffer));
      if( (file_recv_size = recvfrom( sd, buffer, BUFF_SIZE, 0, (struct sockaddr *)&cliaddr, &len )) < 0){
        printf("Timeout File receive End\n");
        break;
      }

    }
    printf("File end: %d\n",i);
    i=0;
    fclose(fp);

    // reset socket timeout opt 
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) < 0) {
      perror("Error");
    }
  }


  close(sd);
  return(0);
}
