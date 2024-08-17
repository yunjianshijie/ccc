#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <optional>
#include <stdlib.h>
#include <unistd.h>
#include "../include/back.hpp"
class Socket_con {
public:
    int lfd;
    Socket_con(int port,std::string ip); // 服务器启动
    ~Socket_con(){
        close(lfd);
    }
    int Connect_clientfd();  // 返回客户端的fd
    std::string get_new(int cfd); // 接收客户端的string
    bool push_new(int cfd, std::string str); // 客户端文件描述符 ，发送string // 发送短的
                                            
};
