/* TCP echo server */

#include <stdio.h>      // printf, perror 등 표준 입출력
#include <stdlib.h>     // exit, atoi 등 표준 라이브러리
#include <string.h>     // memset, bzero, memcpy 등 문자열/버퍼 처리
#include <unistd.h>     // close, read, write 등 시스템 호출
#include <arpa/inet.h>  // htons, htonl, inet_addr 등 네트워크 주소 변환
#include <sys/types.h>  // socket, bind, recvfrom 등 기본 타입
#include <sys/socket.h> // socket, bind, listen, accept, recvfrom, sendto
#include <netinet/in.h> // sockaddr_in 구조체 정의
#include <stdint.h>     // uint8_t, uint16_t, uint32_t 등 고정 폭 정수
#define PORT 12345
#define	MAXLINE	 8192  /* Max text line length */

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    ssize_t n;
    char buf[MAXLINE]; 
    struct sockaddr_in cliaddr, servaddr;

    // 단계 1: 소켓 생성
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // 단계 2: 주소/포트 바인딩
    
    memset(&servaddr, 0, sizeof(servaddr));
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    // 단계 3: 연결 대기
    listen(listenfd, 5);

    // 단계 4: 클라이언트 연결 수락
    socklen_t len = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len);
    printf("Connected from %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

    // 단계 5: 메시지 송수신 (Echo)
    while ((n = read(connfd, buf, MAXLINE)) > 0) {
        write(connfd, buf, n);   // 받은 그대로 Echo
    }

    // 단계 6: 연결 종료 및 소켓 닫기
    close(connfd);
    close(listenfd);
    exit(0);
}



