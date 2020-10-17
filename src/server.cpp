#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

const char* SERVERIP = "127.0.0.1";
const uint16_t PORT = 8888;
const int N_BACKLOG = 5;
const int SIZE_EPOLL_CREATE = 1024; /*没用的*/
const int MAX_EVENTS = 5000; /*有用的*/
const int BUF_SIZE = 1024;

char buffer[BUF_SIZE];

struct UserData {
    UserData(int fd_, const char* buf_ = nullptr, int bufsize_ = 0)
        : fd(fd_), buf(buf_, buf_ + bufsize_) {}
    int fd; /*binding to fd*/
    std::string buf;
};

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
    int server_fd;
    check(server_fd = socket(AF_INET, SOCK_STREAM, 0), "socket() error");

    /*bind()*/
    sockaddr_in server_addr;
    inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr.s_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    check(bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)), "bind() error");

    /*listen*/
    listen(server_fd, N_BACKLOG);

    /*epoll*/

    /*epoll_create()*/
    int epoll_fd;
    check(epoll_fd = epoll_create(SIZE_EPOLL_CREATE), "epoll_create() error");
    
    /*epoll add server_fd*/
    epoll_event ev;
    ev.data.fd = server_fd;
    ev.events = EPOLLIN; /* LT mode */
    check(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev), "epoll add() error");

    /*epoll loop*/
    epoll_event* events = new epoll_event[MAX_EVENTS]; /*like FD_SET*/
    while (true) {

        int nfds;
        check(nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 0), "epoll_wait() error");

        /*fd loop*/
        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == server_fd) {
                printf("accept\n");
                /*accept*/
                sockaddr_in client_addr;
                socklen_t addr_len = sizeof(client_addr);
                int client_fd;
                check(client_fd = accept(server_fd, (sockaddr*)&client_addr, &addr_len), "accept() failed");
                /*epoll add client_fd*/
                ev.data.fd = client_fd;
                ev.events = EPOLLIN;
                check(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev), "epoll add() error");

            } else if (events[i].events & EPOLLIN) {

                // if fd < 0 ? why ?
                int n_recv;
                int client_fd = events[i].data.fd;
                check(n_recv = recv(client_fd, buffer, BUF_SIZE, 0), "recv () error");

                if (n_recv == 0) {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL); /*ev* can be NULL after linux2.6.9*/
                    close(client_fd); /*todo 谁先谁后, 先close还是先del*/
                } else { /*return to client*/
                    /*Bind mydata to event*/
                    UserData* mydata = new UserData(events[i].data.fd, buffer, n_recv);
                    /*epoll register write event*/
                    ev.data.ptr = mydata;
                    ev.events = EPOLLOUT;
                    check(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, mydata->fd, &ev), "epoll mod() error");
                }

            } else if (events[i].events & EPOLLOUT) {

                /*send()*/
                UserData* mydata = (UserData*)events[i].data.ptr;
                check(send(mydata->fd, mydata->buf.c_str(), mydata->buf.length(), 0), "send() error");
                /*epoll register read event*/
                ev.data.fd = mydata->fd;
                ev.events = EPOLLIN;
                check(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, mydata->fd, &ev), "epoll mod() error");
                delete mydata;

            } else {
                /* TODO: error handling*/
                perror("some error.");
            }
        }
    }
}