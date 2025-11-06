/*
 * adder.c - 두 개의 정수를 더하는 CGI 프로그램
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
    char *buf, *p, *arg1_s, *arg2_s;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1=0, n2=0;

    /* Extract the two arguments */
    if ((buf = getenv("QUERY_STRING")) != NULL) {
      // = 을 기준으로 첫 수 시작
      p = strchr(buf, '&');
      *p = '\0';
      if((strchr(buf, '=')) && (strchr(p+1, '='))){
        arg1_s = strchr(buf, '='); // arg1의 전
        strcpy(arg1, arg1_s+1); // arg1

        arg2_s = strchr(p+1, '='); // arg2의 전
        strcpy(arg2, arg2_s+1); // arg2
      }else{
        strcpy(arg1, buf);
        strcpy(arg2, p+1);
      }
      
      n1 = atoi(arg1);  // 문자열(string)을 정수(integer)로 변환
      n2 = atoi(arg2);  // 문자열(string)을 정수(integer)로 변환
    }

    /* Make the response body */
    sprintf(content, "Welcome to add.com: ");
    sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
    sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", 
	    content, n1, n2, n1 + n2);
    sprintf(content, "%sThanks for visiting!\r\n", content);
  
    /* Generate the HTTP response */
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);

    exit(0);
}
/* $end adder */