#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
const char* SERVERIP = "127.0.0.1";
const uint16_t SERVERPORT = 8888;
const int BUF_SIZE = 1024;

void check(int exp, const char* errmsg)
{
    if (exp == -1) {
        perror(errmsg); /* perror() print the errmsg joined with errno*/
        exit(-1);
    }
}

int main()
{
    /*socket()*/
    int client_fd;
    check(client_fd = socket(AF_INET, SOCK_STREAM, 0), "socket() error");
    
    /*connect()*/
    sockaddr_in server_addr;
    inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr.s_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVERPORT);
    
    check(connect(client_fd, (sockaddr*)&server_addr, sizeof(server_addr)), "connect() error");

    /*send()*/
    const char* sendbuf = "wo shi ni die";
    check(send(client_fd, sendbuf, strlen(sendbuf) + 1, 0), "send() error");
    /*recv()*/
    char recvbuf[BUF_SIZE] = {};
    int n_recv;
    check(n_recv = recv(client_fd, recvbuf, BUF_SIZE, 0), "recv() error");

    /*print*/
    printf("%s\n", recvbuf);
}