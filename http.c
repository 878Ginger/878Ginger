#include <csapp.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

void rio_readinitb(rio_t *rp,int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

static ssize_t rio_read(rio_t *rp,char *usrbuf,size_t n) 
{
    int cnt;
    while(rp->rio_cnt <= 0){/*Read the file content if buf is empty*/
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf,sizeof(rp->rio_buf));
        if(rp->rio_cnt < 0){
            if(errno != EINTR){
                return -1;
            }
        }else if(rp->rio_cnt == 0){/*EOF*/
            return 0;
        }else {/*reset buf ptr*/
            rp->rio_bufptr = rp->rio_buf;
        }
    }
    /*when n < rp->rio_cnt, need copy some times */
    cnt = n;
    if(rp->rio_cnt < n){/*one time copy end*/
        cnt = rp->rio_cnt;
    }
    memcpy(usrbuf,rp->rio_bufptr,cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

ssize_t rio_writen(int fd,void *usrbuf,size_t n) 
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;
 
    while(nwritten = write(fd,bufp,nleft) <= 0){
        if(errno == EINTR){
            nwritten = 0;
        }else{
            return -1;
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf,size_t maxlen)
{
    int n,rc;
    char c,*bufp = usrbuf;
    for(n = 1; n < maxlen; n++){
        if (( rc = rio_read(rp,&c,1)) == 1){
            *bufp++ = c;
            if(c == '\n'){
                break;
            }
        }else if (rc == 0){
            if(n == 1){/*EOF no data read*/
                return 0;
            }else{/*EOF some data read*/
                break;
            }
        }else{/*ERROR*/
            return -1;
        }
    }
    *bufp = 0;/*string end sign :'\0'*/
    return n;
}


void process_trans(int fd);
int is_static(char *uri);
void parse_static_uri(char *uri,char *filename);
void parse_dynamic_uri(char *uri,char *filename,char *cgiargs);
void feed_static(int fd,char *filename,int filesize);
void get_filetype(char *filename,char *filetype);
void feed_dynamic(int fd,char *filename,char *cgiargs,char *method,int contentlength);
void error_request(int fd,char *cause,char *errnum,char *shortmsg,char *description);

int main(int argc,char **argv)
{
    int listen_sock,conn_sock,port,clientlen;
    struct sockaddr_in clientaddr;

/*检查输入的命令数*/
if(argc!=2){
fprintf(stderr,"usage:%s<port>\n",argv[0]);
exit(1);
}

     signal(SIGCHLD,sigchld_handler);


port = atoi(argv[1]);

listen_sock=open_listen_sock(port);
while(1){
    clientlen = sizeof(clientaddr);
    conn_sock = accept(listen_sock,(SA*)&clientaddr,&clientlen);
    process_trans(conn_sock);
    close(conn_sock);   /*Parent closes connected socket(important!)*/
         }
}


void process_trans(int fd)
{
int static_flag;
struct stat sbuf;
char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
char filename[MAXLINE],cgiargs[MAXLINE];
rio_t rio;

/*读取命令行和头部*/
rio_readinitb(&rio,fd);
rio_readlineb(&rio,buf,MAXLINE);
sscanf(buf,"%s %s %s",method,uri,version);
if(strcasecmp(method,"GET")&&strcasecmp(method,"POST")){
error_request(fd,method,"501","NOT Implemented","weblet does not implement this method");
return;
}
read_requesthdrs(&rio);
static_flag=is_static(uri);
if(static_flag)
parse_static_uri(uri,filename);
else
parse_dynamic_uri(uri,filename,cgiargs);

if(stat(filename,&sbuf)<0){
error_request(fd,filename,"404","NOT found","weblet could not find this file");
return;
}

if(static_flag){
  if(!(S_ISREG(sbuf.st_mode))||!(S_IRUSR&sbuf.st_mode)){
     error_request(fd,filename,"403","Forboden","weblet is not permtted to read this file");
     return;
  }
  feed_static(fd,filename,sbuf.st_size);
 }
  else{
     if(!(S_ISREG(sbuf.st_mode))||!(S_IXUSR&sbuf.st_mode)){
        error_request(fd,filename,"403","Forbiden","weblet could not run the CGI program");
        return;
     }
     feed_dynamic(fd,filename,cgiargs);
   }
}

int is_static(char *uri)
{
  if(!strstr(uri,"cgi-bin"))
  return 1;
  else
  return 0;
}
void error_request(int fd,char *cause,char *errnum,char *shortmsg,char *description)
{
char buf[MAXLINE],body[MAXBUF];

/*Build the HTTP response body*/
sprintf(body,"<html><title>error request</title>");
sprintf(body,"%s<body bgcolor=""ffffff"">\r\n",body);
sprintf(body,"%s%s:%s\r\n",body,errnum,shortmsg);
sprintf(body,"%s<p>%s:%s\r\n",body,description,cause);
sprintf(body,"%s<hr><em>weblet Web server</em>\r\n",body);

/*Send the HTTP response*/
sprintf(buf,"HTTP/1.0%s%s\r\n",errnum,shortmsg);
rio_writen(fd,buf,strlen(buf));
sprintf(buf,"Content-type:text/html\r\n");
rio_writen(fd,buf,strlen(buf));
sprintf(buf,"Content-length:%d\r\n\r\n",(int)strlen(body));
rio_writen(fd,buf,strlen(buf));
rio_writen(fd,body,strlen(body));
}

void read_requesthdrs(rio_t *rp)
{
char buf[MAXLINE];
rio_readlineb(rp,buf,MAXLINE);
while(strcmp(buf,"\r\n")){
printf("%s",buf);
rio_readlineb(rp,buf,MAXLINE);
}
return;
}

void parse_static_uri(char *uri,char *filename)
{
	char *ptr;
	strcpy(filename,".");
	strcat(filename,uri);
	if(uri[strlen(uri)-1]=='/')
		strcat(filename,"home.html");
}

void parse_dynamic_uri(char *uri,char *filename,char *cgiargs)
{
	char *ptr;
	ptr = index(uri,'?');
	if(ptr){
		strcpy(cgiargs,ptr+1);
		*ptr='\0';
	}
	else
		strcpy(cgiargs,"");
	strcpy(filename,".");
	strcat(filename,uri);
}
void feed_static(int fd,char *filename,int filesize)
{
	int srcfd;
	char *srcp,filetype[MAXLINE],buf[MAXBUF];
	get_filetype(filename,filetype);
	sprintf(buf,"HTTP/1.0 200 OK\r\n");
	sprintf(buf,"%sServer:weblet Web Server\r\n",buf);
	sprintf(buf,"%sContent-length:%d\r\n",buf,filesize);
	sprintf(buf,"%sContent-type:%s\r\n\r\n",buf,filetype);
	rio_writen(fd,buf,strlen(buf));

	srcfd = open(filename,O_RDONLY,0);
	srcp = mmap(0,filesize,PROT_READ,MAP_PRIVATE,srcfd,0);
	close(srcfd);
	rio_writen(fd,srcp,filesize);
	munmap(srcp,filesize);
}
void get_filetype(char *filename,char *filetype)
{
	if(strstr(filename,".html"))
		strcpy(filetype,"text/html");
	else if(strstr(filename,".jpg"))
		strcpy(filetype,"image/jpeg");
	else if(strstr(filename,".mpeg"))
		strcpy(filetype,"video/mpeg");
	else
		strcpy(filetype,"text/html");
}
void feed_dynamic(int fd,char *filename,char *cgiargs)
{
char buf[MAXLINE],*emptylist[] = {NULL};
int pfd[2];

sprintf(buf,"HTTP/1.0 200 OK\r\n");
rio_writen(fd,buf,strlen(buf));
sprintf(buf,"Server:weblet Web Server\r\n");
rio_writen(fd,buf,strlen(buf));

pipe(pfd);
if(fork()==0){
close(pfd[1]);
dup2(pfd[0],STDIN_FILENO);
dup2(fd,STDOUT_FILENO);
execve(filename,emptylist,environ);
}

close(pfd[0]);
write(pfd[1],cgiargs,strlen(cgiargs)+1);
wait(NULL);
close(pfd[1]);
}
