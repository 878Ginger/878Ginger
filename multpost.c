#include <stdio.h>
#include <stdlib.h>
int main(void){
        int len;
        char *lenstr,poststr[20];
        char m[10],n[10],l[255];
        printf("Content-Type:text/html\n\n");
        printf("<HTML>\n");
        printf("<HEAD>\n<TITLE >post Method</TITLE>\n</HEAD>\n");
        printf("<BODY>\n");
        printf("<div style=\"font-size:12px\">\n");
        lenstr = getenv("CONTENT_LENGTH"); 
        char *length = getenv("LENGTH");
        sscanf(length,"%s",l);
        printf("contentlength=%d",atoi(l));
        if(lenstr == NULL)
                printf("<DIV STYLE=\"COLOR:RED\">Error parameters should be entered!</DIV>\n");
        else{
                len=atoi(lenstr);
                fgets(poststr,(len+1),stdin);
                if(sscanf(poststr,"m=%[^&]&n=%s",m,n)!=2){
        //sscanf(method,"%s",f);
        printf("m=%s n=%s",m,n);
        //printf("method=%s",f);
        printf("<DIV STYLE=\"COLOR:RED\">%s</DIV>\n",poststr); 
                        printf("<DIV STYLE=\"COLOR:RED\">Error: Parameters are not right!</DIV>\n");
                }
                else{
                       printf("<DIV STYLE=\"COLOR:GREEN; font-size:15px;font-weight:bold\">m * n = %d</DIV>\n",atoi(m)*atoi(n));
                }
        }
        printf("<HR COLOR=\"blue\" align=\"left\" width=\"100\">");
        printf("<input type=\"button\" value=\"Back CGI\" onclick=\"javascript:window.location='../cgi.html'\">");
        printf("</div>\n");
        printf("</BODY>\n");
        printf("</HTML>\n");
        fflush(stdout);
        return 0;
}
