/* UDP echo server */

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
    size_t n; 
    char buf[MAXLINE]; 
    int sockfd;
    struct sockaddr_in cliaddr;

    // 단계 1: 소켓 생성
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    // 단계 2: 주소/포트 바인딩
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    // 단계 3: 메시지 수신 및 송신
    while (1) {
        socklen_t len = sizeof(cliaddr);
        n = recvfrom(sockfd, buf, MAXLINE, 0, (struct sockaddr*)&cliaddr, &len);
        sendto(sockfd, buf, n, 0, (struct sockaddr*)&cliaddr, len);
    }

    // 단계 6: 연결 종료 및 소켓 닫기
    close(sockfd);
}



