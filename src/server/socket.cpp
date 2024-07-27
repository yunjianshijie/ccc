#include "socket.hpp"
// // 连接客户端
Socket_con::Socket_con(int port, std::string ip) // 服务器启动
{
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0) {
        perror("socket");
        return;
    }
    int opt = 1;
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");

        return;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port); // 大端端口
    addr.sin_addr.s_addr = INADDR_ANY;
    // inet_pton(AF_INET, "192.168.1.100", &addr.sin_addr.s_addr);
    int ret = bind(lfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind");
        return;
    }
    // bind
    ret = listen(lfd, 125);
    if (ret < 0) {
        std::cerr << "Accept failed" << std::endl;
        return;
    }
    std::cout << "服务器启动成功" << std::endl;
    std::cout << "服务器ip为" << ip << std::endl;
}

// 返回客户端的fd
int Socket_con::Connect_clientfd() { // q
    struct sockaddr_in cliaddr;
    int clilen = sizeof(cliaddr);
    int clientfd = accept(lfd, (struct sockaddr *)&cliaddr, (socklen_t *)&clilen);
    if (clientfd < 0) {
        return -1;
    }
    char ip[24] = {0};
    std::string ipp = inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip, sizeof(ip));
    std::cout << "客户端ip为" << ipp << std::endl;
    return clientfd;
}

std::string Socket_con::get_new(int cfd) {
    //
    if (cfd < -1) {
        printf("客户端cfd为-1");
        return "";
    }
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    // 接收消息用客户端文件描述符
    // int len = read(cfd, buf, sizeof(buf));
    int len = recv(cfd, buf, sizeof(buf), 0);
    if (len > 0) {
        printf("客户端say: %s\n", buf);
        std::string ret(buf);
        return ret;
    } else if (len == 0) {
        printf("客户端断开了连接...\n");
        return "";
    } else {
        perror("read");
        return "";
    }
    return "";
}

bool Socket_con::push_new(int cfd, std::string str) { // 客户端文件描述符 ，发送string
                                                      // 发送短的
    // write(cfd, str.c_str(), str.size() + 1);
    int ret = send(cfd, str.c_str(), str.size() + 1, 0);
    if (ret < str.size()) {
        std::cout << "发送未完成" << std::endl;
        return false;
    }
    return true;
}