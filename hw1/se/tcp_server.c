#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1000
void error_handling(char *message);

int main(int argc, char* argv[])
{
  int serv_sock;
  int clnt_sock;

  struct sockaddr_in serv_addr;
  struct sockaddr_in clnt_addr;
  socklen_t clnt_adr_sz;

  char message[BUF_SIZE];
  int str_len, i = 1;

  char file_name[20];
  FILE *fp;     // file pointer

  int file_name_size = 0;
  int total_str_len = 0;

  int count = 0;
  int ret = 0;

  if(argc != 2)
  {
    printf("Usage: %s <port> \n", argv[0]);
    exit(1);
  }

  if((serv_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1){
    error_handling("socket() error");
  }

  memset(&serv_addr, 0, sizeof(serv_addr)); // serv_addr의 사이즈만큼 serv_addr                                                                                                             을 0으로 클리어
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(atoi(argv[1]));

  if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    error_handling("bind() error");

  if((listen(serv_sock, 5)) == -1)
    error_handling("listen() error");

  clnt_adr_sz = sizeof(clnt_addr);

  while(1){
    if((clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_adr_sz)) == -1)
      error_handling("accept() error");

    printf("Connected client %d\n", i++);
    str_len = recv(clnt_sock, &file_name_size, sizeof(file_name_size), 0);  

    // 제목을 읽어서 파일 이름으로 저장하기
    str_len = recv(clnt_sock, file_name, file_name_size, 0); // file name size                                                                                                              만큼만 읽음
    file_name[str_len] = '\0';

    if(str_len == 0)
      error_handling("read() error");

    fp = fopen(file_name, "wb");        // 받은
    if(fp == NULL) error_handling("fopen() error");

    while((str_len = recv(clnt_sock, message, BUF_SIZE, 0)) != 0){
      total_str_len += str_len;
      ret = fwrite(message, 1, str_len, fp);
      memset(message, 0, sizeof(message));
      count++;
    }
    printf("count: %d, total_length: %d\n", count, total_str_len);
    total_str_len = 0;
    count = 0;
    fclose(fp);
    close(clnt_sock);
  }
  close(serv_sock);
  return 0;
}

void error_handling(char *message)
{
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}
