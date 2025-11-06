#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

int parse_uri_for_proxy(const char *uri, char *hostname, char *port, char *path);
void sigchld_handler(int sig);

/* 좀비 자식을 처리하는 핸들러 */
void sigchld_handler(int sig){
  while (waitpid(-1,0,WNOHANG) > 0)
    ;
  return;
}

int main(int argc, char **argv) {
  printf("%s", user_agent_hdr);
  rio_t rio_client, rio_server;
  /* 1. 클라이언트와 연결 */
  int listenfd, connfd, serverfd, n, is_parse;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  char client_hostname[MAXLINE], client_port[MAXLINE], buf[MAXLINE];
  char method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char hostname[MAXLINE], port[MAXLINE], path[MAXLINE];

  if (argc != 2) {
      fprintf(stderr, "usage: %s <port>\n", argv[0]);
      exit(0);
  }
  
  /* 시그널 등록 */
  signal(SIGCHLD, sigchld_handler);

  /* 듣기 소켓 오픈 */
  listenfd = open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(struct sockaddr_storage); 
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
                client_port, MAXLINE, 0);
    printf("Connected to (%s, %s)\n", client_hostname, client_port);
    
    // 클라이언트 소켓 초기화
    Rio_readinitb(&rio_client, connfd);
    
    // (1) 클라이언트 -> 프록시
    Rio_readlineb(&rio_client, buf, MAXLINE);
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);

    if (strcasecmp(method, "GET") && strcasecmp(method,"CONNECT")) {//line:netp:doit:beginrequesterr
      return;
    }

    // 호스트명, 포트, 경로를 추출 (parse_uri_for_proxy)
    is_parse = parse_uri_for_proxy(uri, hostname, port, path);

    if(is_parse){
      sprintf(buf, "!!!!!!!!!!ERROR!!!!!!!!!! GET %s HTTP/1.0\r\n", path);
      Rio_writen(serverfd, buf,strlen(buf));
    }
    /* 2. 서버와 연결 */
    serverfd = open_clientfd(hostname, port);
    
    // 서버 소켓 초기화
    Rio_readinitb(&rio_server, serverfd);

    // (2) 프록시 -> 서버
    // 요청 라인
    sprintf(buf, "GET %s HTTP/1.0\r\n", path);
    Rio_writen(serverfd, buf,strlen(buf));
    // Host 헤더 추가
    sprintf(buf, "Host: %s\r\n", hostname);
    Rio_writen(serverfd, buf, strlen(buf));
    // 프록시에서 필요한 기본 헤더 추가
    Rio_writen(serverfd, "Connection: close\r\n", strlen("Connection: close\r\n"));
    Rio_writen(serverfd, "Proxy-Connection: close\r\n", strlen("Proxy-Connection: close\r\n"));
    Rio_writen(serverfd, "User-Agent: Mozilla/5.0 ...\r\n", strlen("User-Agent: Mozilla/5.0 ...\r\n"));

    // 헤더 종료 (\r\n 두 번)
    Rio_writen(serverfd, "\r\n", strlen("\r\n"));
    
    // (3) 서버 -> 프록시 + 프록시 -> 클라이언트
    while ((n = Rio_readlineb(&rio_server, buf, MAXLINE)) != 0)
      Rio_writen(connfd, buf, n);
    
    Close(serverfd);
    Close(connfd);
  }
  exit(0);
}
/* 
 * parse_uri_for_proxy - HTTP URI 분석
 *          서버 주소 및 포트 추출
 *`strstr()`는 C 언어에서 문자열 안의 특정 부분 문자열을 검색하는 함수
 *`strcpy()`는 한 문자열을 다른 문자열로 복사
 *`strcat()`는 두 문자열을 하나로 합치는(연결하는) 함수 
 *
*/
/* $end echoserverimain */
int parse_uri_for_proxy(const char *uri, char *hostname, char *port, char *path)
{
    const char *host_start;
    const char *port_start;
    const char *path_start;

    // 1. "http://" 제거
    if ((host_start = strstr(uri, "//")) != NULL)
        host_start += 2;
    else
        host_start = uri;

    // 2. '/' 찾기 (path 시작점)
    path_start = strchr(host_start, '/');
    if (path_start != NULL) {
        strcpy(path, path_start);
    } else {
        strcpy(path, "/");
    }

    // 3. ':' 찾기 (port 시작점)
    port_start = strchr(host_start, ':');

    // 4. 포트가 존재하는 경우
    if (port_start != NULL) {
        int host_len = port_start - host_start;
        strncpy(hostname, host_start, host_len);
        hostname[host_len] = '\0'; // strncpy는 널종료 안 해줄 수 있음

        if (path_start)
            strncpy(port, port_start + 1, path_start - (port_start + 1));
        else
            strcpy(port, port_start + 1);

        port[path_start - (port_start + 1)] = '\0';
    }
    // 5. 포트가 없는 경우
    else {
        if (path_start != NULL) {
            int host_len = path_start - host_start;
            strncpy(hostname, host_start, host_len);
            hostname[host_len] = '\0';
        } else {
            strcpy(hostname, host_start);
        }
        strcpy(port, "80");
    }
    printf("Client request: %s\n", uri);
    printf("Parsed host = %s, port = %s, path = %s\n", hostname, port, path);

    return 0;
}

