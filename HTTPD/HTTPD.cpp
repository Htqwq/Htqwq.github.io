#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

#define PRINTF(str) printf("[%s - %d]"#str"=%s", __func__, __LINE__, str);if (str[strlen(str) - 1] != '\n') printf("\n");

void error_die(const char* str) {
    perror(str);
    exit(1);
}

// 实现网络初始化
// 返回值：套接字（服务器端的套接字）
// 参数：port表示端口
//      如果*port的值是0就自动分配一个可用端口
int startup(unsigned short *port) {
    // 网络通信初始化
    WSADATA data;
    int ret = WSAStartup(MAKEWORD(1, 1), &data); // 1.1版本的协议
    if (ret) error_die("WSAStartup");

    // 创建套接字
    int server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1) error_die("socket");

    // 设置端口复用
    int opt = 1;
    ret = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    if (ret == -1) error_die("setsockopt");

    // 配置服务器网络地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_socket));
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(*port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 绑定套接字
    ret = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret < 0) error_die("bind");

    // 动态分配端口
    int nameLen = sizeof(server_addr);
    if (*port == 0) {
        ret = getsockname(server_socket, (struct sockaddr*)&server_addr, &nameLen);
        if (ret < 0) error_die("getsockname");
        *port = server_addr.sin_port;
    }

    // 创建监听队列
    ret = listen(server_socket, 5);
    if (ret < 0) error_die("listen");

    return server_socket;
}

// 从指定的客户端套接字读取一行数据，保存到buff，返回实际读取的字节数
int get_line(int sock, char *buff, int size) {
    char c = 0;
    int i = 0;

    while (i < size - 1 && c != '\n') {
        int n = recv(sock, &c, 1, 0);
        if (n > 0) {
            if (c == '\r') {
                n = recv(sock, &c, 1, MSG_PEEK);
                if (n > 0 && c == '\n') recv(sock, &c, 1, 0);
                else c = '\n';
            }
            buff[i++] = c;
        }
        else c = '\n';
    }

    buff[i] = 0;
    return i;
}

// 向指定的套接字发送一个提示功能还没有实现的错误页面
void unimplement(int client) {
    char buf[1024];
 
    sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Server: HqwqHttpd/0.1\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    strcpy(buf, "<!DOCTYPE html>\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<html lang=\"en\"><head>\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<meta charset=\"UTF-8\">\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<title>Method Not Implemented</title>\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "</head><body>\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<p style=\"text-align: center;\">\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "HTTP request method not supported.\r\n");
    send(client, buf, strlen(buf), 0);
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "</body></html>\r\n");
    send(client, buf, strlen(buf), 0);
}

// 向指定套接字发送一个提示网页不存在的错误页面
void not_found(int client) {
    char buf[1024];
 
    strcpy(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "Server: HqwqHttpd/0.1\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    strcpy(buf, "<!DOCTYPE html>\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<html lang=\"en\"><head>\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<meta charset=\"UTF-8\">\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<title>Not Found</title>\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "</head><body>\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<p style=\"text-align: center;\">\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "The server could not fulfill\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "your request because the resource specified\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "is unavailable or nonexistent.\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "</body></html>\r\n");
    send(client, buf, strlen(buf), 0);
}

void bad_request(int client) {
    char buf[1024];
 
    strcpy(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "Server: HqwqHttpd/0.1\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    strcpy(buf, "<!DOCTYPE html>\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<html lang=\"en\"><head>\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<meta charset=\"UTF-8\">\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<title>Bad Request</title>\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "</head><body>\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<p style=\"text-align: center;\">\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "Your browser sent a bad request, \r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "such as a POST without a Content-Length.<br>\r\n");
    send(client, buf, sizeof(buf), 0);
    strcpy(buf, "</body></html>\r\n");
    send(client, buf, strlen(buf), 0);
}

void cannot_execute(int client) {
    char buf[1024];
 
    strcpy(buf, "HTTP/1.0 500 Internal Server Error\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "Server: HqwqHttpd/0.1\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    strcpy(buf, "<!DOCTYPE html>\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<html lang=\"en\"><head>\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<meta charset=\"UTF-8\">\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<title>Cannot Execute</title>\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "</head><body>\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "<p style=\"text-align: center;\">\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "Error prohibited CGI execution.\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "</body></html>\r\n");
    send(client, buf, strlen(buf), 0);
}

// 发送响应包的头信息
void headers(int client, const char* type) {
    char buff[1024];

    sprintf(buff, "HTTP/1.1 200 OK\r\n");
    send(client, buff, strlen(buff), 0);
    sprintf(buff, "Server: HqwqHttpd/0.1\r\n");
    send(client, buff, strlen(buff), 0);
    PRINTF(type);
    sprintf(buff, "Content-type: %s\r\n", type);
    send(client, buff, strlen(buff), 0);
    sprintf(buff, "\r\n");
    send(client, buff, strlen(buff), 0);
}

const char* getHeadType(const char* fileName){
    const char* ret = "text/html";
    const char* p = strrchr(fileName, '.');
    if (!p) return ret;

    p++;
    if (!strcmp(p, "css")) ret = "text/css";
    else if (!strcmp(p, "jpg") || !strcmp(p, "jpeg")) ret = "image/jpeg";
    else if (!strcmp(p, "png")) ret = "image/png";
    else if (!strcmp(p, "webp")) ret = "image/webp";
    else if (!strcmp(p, "js")) ret = "application/x-javascript";

    return ret;
}

void cat(int client, FILE* resource) {
    char buff[4096];
    int count = 0;

    while (1) {
        int ret = fread(buff, sizeof(char), sizeof(buff), resource);
        if (ret <= 0) break;
        send(client, buff, ret, 0);
        count += ret;
    }
    printf("一共发送 %d 字节给浏览器\n", count);
}

// 发送资源给客户端
void server_file(int client, const char* fileName) {
    FILE *resource = NULL;
    if (!strcmp(getHeadType(fileName), "text/html") || !strcmp(getHeadType(fileName), "text/css") || !strcmp(getHeadType(fileName), "application/x-javascript")) resource = fopen(fileName, "r");
    else resource = fopen(fileName, "rb");
    //resource = fopen(fileName, "r");
    if (resource == NULL) {
        not_found(client);
    }
    else {
        // 正式发送资源给浏览器
        headers(client, getHeadType(fileName));
        // 发送请求的资源信息
        cat(client, resource);
        printf("资源发送完毕!\n");
    }
    fclose(resource);
}

void execute_cgi(int client, const char *path, const char *method, const char *query_string) {
    char buff[1024];
    int numchars = 1;
    int content_length = -1;
    DWORD size;

    buff[0] = 'A';buff[1] = '\0';
    if (strcasecmp(method, "GET") == 0) {
        // 请求包的剩余数据读取完毕
        while ((numchars > 0) && strcmp("\n", buff)) numchars = get_line(client, buff, sizeof(buff));
    }
    else {
        //numchars = get_line(client, buff, sizeof(buff));
        while ((numchars > 0) && strcmp("\n", buff)) {
            buff[15] = '\0';
            if (strcasecmp(buff, "Content-Length:") == 0)
                content_length = atoi(&(buff[16])); //记录 body 的长度大小
            numchars = get_line(client, buff, sizeof(buff));
        }
        
        //如果 http 请求的 header 没有指示 body 长度大小的参数，则报错返回
        if (content_length == -1) {
            bad_request(client);
            return;
        }
    }

    sprintf(buff, "HTTP/1.1 200 OK\r\n");
    send(client, buff, strlen(buff), 0);
    sprintf(buff, "Server: HqwqHttpd/0.1\r\n");
    send(client, buff, strlen(buff), 0);
    // sprintf(buff, "Content-type: text/html\r\n\r\n");
    // send(client, buff, strlen(buff), 0);
        
     // 管道的句柄
    HANDLE output[2], input[2];  
    // 管道的属性
    SECURITY_ATTRIBUTES la;
    la.nLength = sizeof(la);
    la.bInheritHandle = true;
    la.lpSecurityDescriptor = 0; 

    if (CreatePipe(&output[0], &output[1], &la, 0) == false) {
        cannot_execute(client);
        return;
    }
    if (CreatePipe(&input[0], &input[1], &la, 0) == false) {
        cannot_execute(client);
        return;
    }

    //如果是 POST 方法的话就继续读 body 的内容,并写入input管道
    if (strcasecmp(method, "POST") == 0)
    for (int i = 0; i < content_length; i++) {
        char c;
        recv(client, &c, 1, 0);
        buff[i] = c;
    }
    buff[content_length] = 0;
    WriteFile(input[1], buff, strlen(buff) + 1, &size, NULL);

    char meth_env[255];
    char query_env[255];
    char length_env[255];

    //构造一个环境变量
    sprintf(meth_env, "REQUEST_METHOD=%s", method);
    //将这个环境变量加进子进程的运行环境中
    putenv(meth_env);

    //根据http 请求的不同方法，构造并存储不同的环境变量
    if (strcasecmp(method, "GET") == 0) {
        sprintf(query_env, "QUERY_STRING=%s", query_string);
        putenv(query_env);
    }
    else {
        sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
        putenv(length_env);
    }
    
    char cmd[] = "D:\\Program\\VScode\\emm\\htdocs\\test.cgi";
    // 子进程启动属性
    STARTUPINFO si = {0};
    si.cb = sizeof(si);
    si.hStdInput = input[0];
    si.hStdOutput = output[1];
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi = {0};

    // 创建子进程
    if (CreateProcess(NULL,  cmd, 0, 0, TRUE, 0, 0, 0, &si, &pi) == false) {
        cannot_execute(client);
        return;
    }

    //读取子进程的输出，并发送到客户端
    ReadFile(output[0], buff, sizeof(buff), &size, NULL);
    buff[size] = 0;
    for (int i=0; i < size; i++) {
        char c = buff[i];
        if (c == '\r') continue;
        if (c == '\n') {
            c = '\r';
            send(client, &c, 1, 0);
            c = '\n';
            send(client, &c, 1, 0);
        }
        else send(client, &c, 1, 0);
    }

    // 关闭管道
    CloseHandle(input[1]);
    CloseHandle(input[0]);
    CloseHandle(output[1]);
    CloseHandle(output[0]);

    WaitForSingleObject(pi.hProcess, INFINITE);
}

// 处理用户请求的线程函数
DWORD WINAPI accept_request(LPVOID arg) {
    char buff[1024];
    int cgi = 0;

    int client = (SOCKET)arg;   //客户端套接字

    // 读取一行数据
    // "GET / HTTP/1.1\n"
    int numchars = get_line(client, buff, sizeof(buff));
    PRINTF(buff);

    char method[255];
    int j = 0, i = 0;
    while (!isspace(buff[j]) && i < sizeof(method) - 1 && j < sizeof(buff) - 1) {
        method[i++] = buff[j++];
    }
    method[i] = 0;
    PRINTF(method);

    // 检查请求的方法本服务器是否支持
    if (stricmp(method, "GET") && stricmp(method, "POST")) {
        // 向浏览器返回一个错误提示页面
        unimplement(client);
        return 0;
    }

    //如果是 POST 方法就将 cgi 标志变量置一(true)
    if (strcasecmp(method, "POST") == 0) cgi = 1;

    //解析资源文件的路径
    char url[255];  //存放请求的资源的完整路径
    i = 0;
    while (isspace(buff[j]) && j < sizeof(buff) - 1) j++;
    while (!isspace(buff[j]) && i < sizeof(url) - 1 && j < sizeof(buff) - 1) {
        url[i++] = buff[j++];
    }
    url[i] = 0;
    PRINTF(url);

    //如果这个请求是一个 GET 方法的话
    char *query_string = NULL;
    if (strcasecmp(method, "GET") == 0) {
        //用一个指针指向 url
        query_string = url;
        
        //去遍历这个 url，跳过字符 ？前面的所有字符，如果遍历完毕也没找到字符 ？则退出循环
        while ((*query_string != '?') && (*query_string != '\0')) query_string++;
        
        //退出循环后检查当前的字符是 ？还是字符串(url)的结尾
        if (*query_string == '?') {
            //如果是 ？ 的话，证明这个请求需要调用 cgi，将 cgi 标志变量置一(true)
            cgi = 1;
            //从字符 ？ 处把字符串 url 给分隔会两份
            *query_string = '\0';
            //使指针指向字符 ？后面的那个字符
            query_string++;
        }
    }

    //获取资源完整路径
    char path[512] = "";
    sprintf(path, "D:/Program/VScode/emm/htdocs%s", url);
    if (path[strlen(path) - 1] == '/') strcat(path, "index.html");
    PRINTF(path);
    
    struct stat status;
    if (stat(path, &status) == -1) {
        // 请求包的剩余数据读取完毕
        while (numchars > 0 && strcmp(buff, "\n")) numchars = get_line(client, buff, sizeof(buff));
        not_found(client);
    }
    else {
        //如果这个文件是个目录，那就需要再在 path 后面拼接一个"/index.html"的字符串
        if ((status.st_mode & S_IFMT) == S_IFDIR) {
            strcat(path, "/index.html");
        }
        //如果这个文件是一个可执行文件，不论是属于用户/组/其他这三者类型的，就将 cgi 标志变量置一
        if ((status.st_mode & S_IXUSR) || (status.st_mode & S_IXGRP) || (status.st_mode & S_IXOTH)) cgi = 1;
        
        if (!cgi) {
            // 请求包的剩余数据读取完毕
            while (numchars > 0 && strcmp(buff, "\n")) numchars = get_line(client, buff, sizeof(buff));
            server_file(client, path);
        }
        else execute_cgi(client, path, method, query_string);
    }

    closesocket(client);
    return 0;
} 

int main() {
    //10.44.61.17
    unsigned short port = 80;
    int server_sock = startup(&port);
    printf("httpd服务已经启动,正在监听 %d 端口...\n", port);

    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    while (1) {
        // 阻塞式等待用户通过浏览器发起访问
        long long client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_sock == -1) error_die("accept");

        // 使用client_sock对用户进行访问
        // 创建新的线程
        DWORD threadId = 0;
        CreateThread(0, 0, accept_request, (void*)client_sock, 0, &threadId);
    }

    closesocket(server_sock);
    return 0;
}