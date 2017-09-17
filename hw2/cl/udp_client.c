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
#define NAME_BUFF_SIZE 256
#define BUFF_SIZE 1024

int main(int argc, char **argv){
  static const char PRESET_NAME[] = "FILE:";
  static const char PRESET_DATA[] = "DATA:";

  char name_buffer[NAME_BUFF_SIZE]="", file_buffer[BUFF_SIZE]="", file_temp_buffer[BUFF_SIZE]="";
  int sockfd,connfd,len;
  ssize_t total_file_size, file_size;
  FILE *fp;
  int i=0;

  struct sockaddr_in servaddr,cliaddr;

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
  sockfd = socket( PF_INET, SOCK_DGRAM, 0 );

  if(sockfd==-1){
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

  // Reset Offset
  fseek(fp, 0, SEEK_END);
  total_file_size=ftell(fp);
  fseek(fp, 0, SEEK_SET);

  // Reset arrays
  memset(file_buffer, 0, sizeof(file_buffer));

  // Send data
  while( !feof(fp) ){
    file_size = fread(file_temp_buffer, sizeof(char), sizeof(file_temp_buffer)-strlen(PRESET_DATA), fp);
    memcpy( file_buffer, PRESET_DATA, sizeof(PRESET_DATA) );
    memmove( &file_buffer[sizeof(PRESET_DATA)-1], &file_temp_buffer[0], sizeof(file_temp_buffer)-strlen(PRESET_DATA) );
    sendto( sockfd, file_buffer, file_size+strlen(PRESET_DATA), 0, (struct sockaddr *)&servaddr, sizeof(struct sockaddr) );
    memset(file_buffer, 0, sizeof(file_buffer));
    i++;
  }
  printf("File end: %d\n",i);

  // Send file name to Server
  memset(name_buffer, 0, sizeof(name_buffer));
  snprintf( name_buffer, sizeof(name_buffer), "%s%s", PRESET_NAME, argv[3] );
  sendto( sockfd, name_buffer, strlen(name_buffer), 0, (struct sockaddr *)&servaddr, sizeof(struct sockaddr) );

  //close client side connection
  fclose(fp);
  close(sockfd);

  return(0);
}
