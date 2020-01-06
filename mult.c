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

int main(void)
{
        char *data;
        char m[10],n[10],ll[10];
        printf("Content-Type:text/html\n\n");
        printf("<HTML>\n");
        printf("<HEAD>\n<TITLE >Get Method</TITLE>\n</HEAD>\n");
        printf("<BODY>\n");
        printf("<div style=\"font-size:12px\">\n");
        data = getenv("QUERY_STRING");
        char *length = getenv("LENGTH");
        sscanf(length,"%s",ll);
        printf("contentlength=%d",atoi(ll));
        if(sscanf(data,"m=%[^&]&n=%s",m,n)!=2){
                printf("<DIV STYLE=\"COLOR:RED\">Error parameters should be entered!</DIV>\n");
        }
        else{
               printf("<DIV STYLE=\"COLOR:GREEN; font-size:15px;font-weight:bold\">a * b = %d</DIV>\n",atoi(m)*atoi(n));
        }
        printf("<HR COLOR=\"blue\" align=\"left\" width=\"100\">");
        printf("<input type=\"button\" value=\"Back CGI\" onclick=\"javascript:window.location='../cgi.html'\">");
        printf("</div>\n");
        printf("</BODY>\n");
        printf("</HTML>\n");
        return 0;
}
