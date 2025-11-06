/*
 * echo - read and echo text lines until client closes connection
 */
/* $begin echo */
#include "csapp.h"

void echo(int connfd) 
{
    size_t n; 
    char buf[MAXLINE]; 
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) { //line:netp:echo:eof
        // Ctrl + Z 후 Enter를 해도 멈추지 않아서 추가
        // exit 입력 시 종료
        if(strcmp(buf, "exit\n") == 0){
            break;
        }
        printf("server received %d bytes\n", (int)n);
        Rio_writen(connfd, buf, n);
    }
}
/* $end echo */