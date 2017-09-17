#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 500

void error_handling(char *message);

int main(int argc, char* argv[])
{
  int sock;
  struct sockaddr_in serv_addr;
  char message[BUF_SIZE];
  int str_len, recv_len, recv_cnt;
  char file_name[20];

  int total_write = 0;
  int file_size = 0, file_name_size = 0;
  int count = 0;
  ssize_t file_read_size;

  if(argc != 4)
  {
    printf("Usage: %s <IP> <port> <file name> \n", argv[0]);
    exit(1);
  }
  memset(file_name, 0, BUF_SIZE);
  strcpy(file_name, argv[3]);

  // 소켓 생성, TCP로 생성함 
  if((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1){
    error_handling("socket() error");
  }

  memset(&serv_addr, 0, sizeof(serv_addr)); // serv_addr의 사이즈만큼 serv_addr을 0으로 클리어 
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
  serv_addr.sin_port = htons(atoi(argv[2]));

  if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    error_handling("connect() error");
  else
    printf("연결 되었습니다.\n");

  // file open 
  FILE *fp = fopen(file_name, "rb");
  if(fp == NULL) error_handling("fopen() error");

  // file size check 
  file_name_size = strlen(file_name);

  // file name size 전송 
  str_len = send(sock, &file_name_size, sizeof(file_name_size), 0);
  // file name 전송 
  str_len = send(sock, file_name, strlen(file_name), 0);

  // 파일에서 내용을 읽어서 서버로 보낼 것 
  while( !feof(fp) ){
    file_read_size = fread(message, 1, sizeof(message), fp);
    str_len = send(sock, message, file_read_size, 0);
    total_write += str_len;
    count++;
  }

  printf("count:%d, total_length: %d\n", count, total_write);
  fclose(fp);
  close(sock);
  return 0;
}

void error_handling(char *message)
{
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}
