#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define MAXLINE 1024
#define TOTAL_MSG 5      // 총 전송 메시지 수
#define INTERVAL_US 10000  // 메시지 간 전송 간격(마이크로초, 10ms)
struct timeval tv;

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char sendbuf[MAXLINE], recvbuf[MAXLINE];
    socklen_t len = sizeof(servaddr);
    int i, n;

    // 1. UDP 소켓 생성
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error");
        exit(1);
    }

    // 2. 서버 주소 설정
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);

    // 3. 연속 메시지 전송
    for (i = 1; i <= TOTAL_MSG; i++) {
        // 메시지에 순번 포함
        gettimeofday(&tv, NULL);
        snprintf(sendbuf, MAXLINE, "Seq %d Time %ld.%06ld", i, (long)tv.tv_sec, (long)tv.tv_usec);

        // 서버로 전송
        sendto(sockfd, sendbuf, strlen(sendbuf), 0,
               (struct sockaddr*)&servaddr, len);

        // 서버 Echo 수신
        n = recvfrom(sockfd, recvbuf, MAXLINE, 0, NULL, NULL);
        recvbuf[n] = '\0';

        printf("Sent: %s, Received: %s\n", sendbuf, recvbuf);

        // 전송 간격
        usleep(INTERVAL_US);
    }

    close(sockfd);
    return 0;
}
