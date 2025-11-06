#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define MAXLINE 1024
#define TOTAL_MSG 5      // 전송 메시지 수
#define INTERVAL_US 10000  // 메시지 간 간격 (10ms)

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char sendbuf[MAXLINE], recvbuf[MAXLINE];
    int n, i;

    // 1. TCP 소켓 생성
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(1);
    }

    // 2. 서버 주소 설정
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);

    // 3. 서버에 연결
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        exit(1);
    }

    printf("Connected to %s:%d\n", SERVER_IP, SERVER_PORT);

    // 4. 연속 메시지 전송
    for (i = 1; i <= TOTAL_MSG; i++) {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        // 순번 + timestamp 포함
        snprintf(sendbuf, MAXLINE, "Seq %d Time %ld.%06ld", i, (long)tv.tv_sec, (long)tv.tv_usec);

        // 서버로 전송
        write(sockfd, sendbuf, strlen(sendbuf));

        // 서버 Echo 수신
        n = read(sockfd, recvbuf, MAXLINE);
        recvbuf[n] = '\0';

        printf("Sent: %s | Received: %s\n", sendbuf, recvbuf);

        // 메시지 간 간격
        usleep(INTERVAL_US);
    }

    close(sockfd);
    return 0;
}
