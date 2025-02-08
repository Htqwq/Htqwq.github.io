#include<bits/stdc++.h>
using namespace std;

int main(){
    printf("Content-type: text/html\r\n\r\n");
    printf("<!DOCTYPE html>\r\n");
    printf("<html lang=\"en\">\r\n");
    printf("<head><meta charset=\"UTF-8\">\r\n");
    printf("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n");
    printf("<title>hello world</title></head><body>\r\n");
    printf("\r\n");
    printf("\r\n");
    
    char buff[1024];
    char* value = getenv("CONTENT_LENGTH");
    if (value == NULL) {
        printf("<h1>Post for error\r\n");
    }
    else {
        // printf("<P>CONTENT_LENGTH = %s\r\n", value);
        long len = atoi(value);
        fgets(buff, len + 1, stdin);
        //printf("%s\r\n", buff);
        char* color = buff;
        while (*color != '=') color++;
        color++;
        printf("<h1 style=\"color: %s;\">hello world</h1>\r\n", color);
    }
    
    printf("</body></html>\r\n");
    return 0;
}