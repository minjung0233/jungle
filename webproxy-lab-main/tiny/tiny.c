/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
/* 좀비 자식을 처리하는 핸들러 */
void sigchld_handler(int sig){
  while (waitpid(-1,0,WNOHANG) > 0)
    ;
  return;
}

int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  /* 시그널 등록 */
  signal(SIGCHLD, sigchld_handler);

  /* 듣기 소켓 오픈 */
  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    /* 연결 요청 접수 */
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    /* 트랜잭션 수행 */
    doit(connfd);   // line:netp:tiny:doit

    Close(connfd);  // line:netp:tiny:close
  }
}

/* $end tinymain */

/*
 * doit - 한 개의 HTTP 트랜잭션을 처리한다.
 */
/* $begin doit */
void doit(int fd) 
{
  int is_static;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  /* Read request line and headers */
  Rio_readinitb(&rio, fd);
  if (!Rio_readlineb(&rio, buf, MAXLINE))  //line:netp:doit:readrequest
    return;
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);       //line:netp:doit:parserequest
  /* GET 요청 */
  if (strcasecmp(method, "GET") && strcasecmp(method,"HEAD")) {//line:netp:doit:beginrequesterr
    clienterror(fd, method, "501", "Not Implemented",
                "Tiny does not implement this method");
    return;
  }                                                    //line:netp:doit:endrequesterr
  else if(!strcasecmp(method,"HEAD")){
    get_filetype(filename, filetype);    //line:netp:servestatic:getfiletype
    sprintf(buf, "HTTP/1.0 200 OK\r\n"); //line:netp:servestatic:beginserve
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n", filesize);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: %s\r\n\r\n", filetype);
    Rio_writen(fd, buf, strlen(buf));
    return;
  }
  /* 요청 헤더 버림 */
  read_requesthdrs(&rio);                              //line:netp:doit:readrequesthdrs

  /* Parse URI from GET request */
  is_static = parse_uri(uri, filename, cgiargs);       //line:netp:doit:staticcheck
  if (stat(filename, &sbuf) < 0) {                     //line:netp:doit:beginnotfound
    clienterror(fd, filename, "404", "Not found",
          "Tiny couldn't find this file");
    return;
  }                                                    //line:netp:doit:endnotfound

  if (is_static) { /* Serve static content */          
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) { //line:netp:doit:readable
      clienterror(fd, filename, "403", "Forbidden",
      "Tiny couldn't read the file");
      return;
    }
    serve_static(fd, filename, sbuf.st_size);        //line:netp:doit:servestatic
  }
  else { /* Serve dynamic content */
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) { //line:netp:doit:executable
      clienterror(fd, filename, "403", "Forbidden",
      "Tiny couldn't run the CGI program");
      return;
    }
    serve_dynamic(fd, filename, cgiargs);            //line:netp:doit:servedynamic
  }
}
/* $end doit */

/*
 * read_requesthdrs - 요청 헤더 읽고 무시한다
 * (tiny는 요청헤더내의 세부 정보를 사용하지 않는다.)
 */
/* $begin read_requesthdrs */
void read_requesthdrs(rio_t *rp) 
{
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  printf("%s", buf);
  while(strcmp(buf, "\r\n")) {          //line:netp:readhdrs:checkterm
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}
/* $end read_requesthdrs */

/*
 * parse_uri - HTTP URI를 분석한다.
 *             return 동적 콘텐츠이면 0, 정적 콘텐츠이면 1
 */
/* $begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs) 
{
  char *ptr;
  /* Static content */
  if (!strstr(uri, "cgi-bin")) {                      //line:netp:parseuri:isstatic
    strcpy(cgiargs, "");                             //line:netp:parseuri:clearcgi
    strcpy(filename, ".");                           //line:netp:parseuri:beginconvert1

    strcat(filename, uri);                           //line:netp:parseuri:endconvert1
    if (uri[strlen(uri)-1] == '/')                   //line:netp:parseuri:slashcheck
      strcat(filename, "home.html");               //line:netp:parseuri:appenddefault
    return 1;
  }
  else {  /* Dynamic content */                        //line:netp:parseuri:isdynamic
    ptr = index(uri, '?');                           //line:netp:parseuri:beginextract
    if (ptr) {
	    strcpy(cgiargs, ptr+1);
	    *ptr = '\0';
	  }
	  else {
	    strcpy(cgiargs, "");                         //line:netp:parseuri:endextract
    }
    strcpy(filename, ".");                           //line:netp:parseuri:beginconvert2
    strcat(filename, uri);                           //line:netp:parseuri:endconvert2
	  return 0;
  }
}
/* $end parse_uri */

/*
 * serve_static - 정적 콘텐츠를 클라이언트에게 서비스한다
 * get_filetype를 이용해 정적 콘텐츠 타입마다 나눔
 */
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize)
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  /* Send response headers to client */
  get_filetype(filename, filetype);    //line:netp:servestatic:getfiletype
  sprintf(buf, "HTTP/1.0 200 OK\r\n"); //line:netp:servestatic:beginserve
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n", filesize);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: %s\r\n\r\n", filetype);
  Rio_writen(fd, buf, strlen(buf));    //line:netp:servestatic:endserve

  /* Send response body to client */
  srcfd = Open(filename, O_RDONLY, 0); //line:netp:servestatic:open

  // srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); //line:netp:servestatic:mmap

  srcp = malloc(filesize);    // 파일 크기만큼 메모리 확보
  rio_readn(srcfd, srcp, filesize);    // 파일 내용 읽어오기
  Close(srcfd);                       //line:netp:servestatic:close
  Rio_writen(fd, srcp, filesize);     // 클라이언트로 전송
  free(srcp);

  // Munmap(srcp, filesize);             //line:netp:servestatic:munmap
}

/*
 * get_filetype - 파일 타입 검사
 */
void get_filetype(char *filename, char *filetype) 
{
  if (strstr(filename, ".html"))
	  strcpy(filetype, "text/html");
  else if (strstr(filename, ".gif"))
	  strcpy(filetype, "image/gif");
  else if (strstr(filename, ".png"))
	  strcpy(filetype, "image/png");
  else if (strstr(filename, ".jpg"))
	  strcpy(filetype, "image/jpeg");
  else if (strstr(filename, ".mp4"))
    strcpy(filetype, "video/mp4");
  else if (strstr(filename, ".mpg"))
    strcpy(filetype, "video/mpg");
  else
	  strcpy(filetype, "text/plain");
}  
/* $end serve_static */

/*
 * serve_dynamic - 동적 콘텐츠를 클라이언트에 제공한다.
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs) 
{
  
  char buf[MAXLINE], *emptylist[] = { NULL };

  /* Return first part of HTTP response */
  sprintf(buf, "HTTP/1.0 200 OK\r\n"); 
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  /* 시그널 등록(자식을 핸들러로 청소) */
  signal(SIGCHLD,sigchld_handler);

  if (Fork() == 0) { /* Child */ //line:netp:servedynamic:fork
    /* Real server would set all CGI vars here */
    setenv("QUERY_STRING", cgiargs, 1); //line:netp:servedynamic:setenv
    Dup2(fd, STDOUT_FILENO);         /* Redirect stdout to client */ //line:netp:servedynamic:dup2
    Execve(filename, emptylist, environ); /* Run CGI program */ //line:netp:servedynamic:execve
  }
  // Wait(NULL); /* Parent waits for and reaps child */ //line:netp:servedynamic:wait
}
/* $end serve_dynamic */

/*
 * clienterror - 에러 메시지를 클라이언트에게 보낸다.
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
  char buf[MAXLINE];

  /* Print the HTTP response headers */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n\r\n");
  Rio_writen(fd, buf, strlen(buf));

  /* Print the HTTP response body */
  sprintf(buf, "<html><title>Tiny Error</title>");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
  Rio_writen(fd, buf, strlen(buf));
}
/* $end clienterror */