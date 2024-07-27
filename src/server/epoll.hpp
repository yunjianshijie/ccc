#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <vector>

class Epoll
{
private:
    /* data */
    std::vector<int> client_fds; // 客户端vector
    int epoll_fd; 
public:
    Epoll(int num); //epoll最多的监听个数
    ~Epoll();
    void epoll_do(); //epoll循环
};




//将 服务器socket加入 epoll监听

bool s_add_epoll(struct epoll_event* event,int epoll_fd,int server_fd);

//将新请求加入epoll监听
