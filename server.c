#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <time.h>

#define SERVER_PORT "8083"
#define BACKLOG 1

int main() 
{
    int sock_fd, new_fd;
    struct addrinfo hints, *res;
    struct sockaddr_storage their_addr;
    socklen_t addr_size;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int getaddrinfo_status = getaddrinfo(NULL, SERVER_PORT, &hints, &res);

    if(getaddrinfo_status != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_status));
        exit(EXIT_FAILURE);
    }

    sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    
    if(sock_fd == -1) 
    {
        perror("Error creating the file descriptor using socket()");
        exit(EXIT_FAILURE);
    }

    int bind_status = bind(sock_fd, res->ai_addr, res->ai_addrlen);

    if(bind_status == -1) 
    {
        perror("Error binding socket to port using bind()");
        exit(EXIT_FAILURE);
    }

    int listen_status = listen(sock_fd, BACKLOG);

    if(listen_status == -1) 
    {
        perror("Error calling listen()");
        exit(EXIT_FAILURE);
    }

    addr_size = sizeof their_addr;
    new_fd = accept(sock_fd, (struct sockaddr*) &their_addr, &addr_size);

    if(new_fd == -1) 
    {
        perror("Error calling accept()");
        exit(EXIT_FAILURE);
    }

    FILE* fptr;
    fptr = fopen("index.html", "r");

    if(fptr == NULL)
    {
        printf("File is not opened\n");
    }

    fseek(fptr, 0, SEEK_END);
    int f_size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    // Building Response Headers
    char status_line[] = "HTTP/1.1 200 OK\r\n";
    
    char date[64];
    time_t now = time(NULL);
    struct tm* tm_info = gmtime(&now);
    strftime(date, sizeof(date), "Date: %a, %d %b %Y %H:%M:%S GMT\r\n", tm_info);

    char content_type[] = "Content-Type: text/html\r\n";
    
    char content_length[100];
    snprintf(content_length, sizeof(content_length), "Content-Length: %d\r\n", f_size);
    char connection[] = "Connection: close\r\n";
    char blank_line[] = "\r\n";
    
    char response_headers[250];
    snprintf(response_headers, sizeof(response_headers), "%s%s%s%s%s%s", status_line, date, content_type, content_length, connection, blank_line);
    int headers_sent = send(new_fd, response_headers, strlen(response_headers), 0);
    
     
    if(headers_sent == -1) 
    {
        perror("Error sending headers!");
        exit(EXIT_FAILURE);
    }
 
    char data[1024];
    int len, bytes_sent;

    // Reading index.html and sending packets via tcp to client
    while(fgets(data, sizeof(data), fptr) != NULL)
    {
        bytes_sent = send(new_fd, data, strlen(data), 0);
    } 

    if(bytes_sent == -1) 
    {
        perror("Error sending bytes packet!");
        exit(EXIT_FAILURE);
    }

    while(1) 
    {
        printf("Server is running....\n");
        sleep(2);
    }

    close(sock_fd);
    return 0;
}
