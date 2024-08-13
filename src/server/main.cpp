#include <arpa/inet.h>
#include "epoll.hpp"
#include "socket.hpp"
#include "chuli/chuli.hpp"
#include "threadpool.hpp"
// #include <shared_mutex>
#include <functional>
#include "tp.h"
#include <hiredis/hiredis.h>
#include <fcntl.h>
#include <netinet/tcp.h>
// void configureKeepAlive(int curfd);
void work(int client_fd, Redis &redis, int epoll_fd);
int main(int argc, char *argv[]) {
    Redis redis;         // 先启动redis
    ThreadPool pool(24); // 线程池
    int port = HAND;     // 默认端口
    std::string serverAddr = ServerAddr;
    if (argc == 2) {
        port = atoi(argv[1]);
    }
    Socket_con server(port, serverAddr);
    //
    int epoll_fd = epoll_create1(0); // 创建epoll实例
    // std::cout << "2" << std::endl;
    if (epoll_fd < 0) {
        perror("epoll_create error");
        std::cout << "3" << std::endl;
        return -1;
    }
    // std::cout << "4" << std::endl;
    struct epoll_event ev;
    ev.events = EPOLLIN || EPOLLET; // 启用ET模式
    ev.data.fd = server.lfd;        // 监听文件描述符
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server.lfd, &ev);
    // std::cout << "5" << std::endl;
    if (ret < 0) {
        perror("epoll_ctl error");
        return -1;
    }
    // std::cout << "6" << std::endl;
    //  循环监听

    while (1) {
        struct epoll_event evs[MAX_EVENTS];
        // std::cout << "7" << std::endl;
        int nfds = epoll_wait(epoll_fd, evs, MAX_EVENTS, -1); // 等待事件发生
        // nfds 这次调用中就绪的文件描述符的数量。
        // std::cout << "8" << std::endl;
        if (nfds < 0) {
            std::cerr << "epoll_wait failed" << std::endl;
            continue;
        }

        for (int i = 0; i < nfds; i++) {
            // 这里循环监听嘛
            int keep_alive = 1;
            int keep_idle = 3;
            int keep_interval = 1;
            int keep_count = 30;

            int fd = evs[i].data.fd;
            if (fd == server.lfd) {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int client_fd = accept(server.lfd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (client_fd < 0) {
                    perror("accept error");
                    continue;
                }
                // 加入epoll
                // // 设置心跳超时时间
                // struct timeval timeout;
                // timeout.tv_sec = TIMEOUT_SECONDS;
                // timeout.tv_usec = 0;
                // if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
                //     perror("setsockopt");
                //     close(client_fd);
                //     continue;
                // }
                //
                struct epoll_event ev_client;          // 客户端连接
                ev_client.events = EPOLLIN || EPOLLET; // 启用ET模式
                ev_client.data.fd = client_fd;
                // epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev_client);
                // 非阻塞
                std::cout << "有新的连接   " << client_fd << std::endl;
                int flag = fcntl(client_fd, F_GETFL, 0);
                fcntl(client_fd, F_SETFL, flag | O_NONBLOCK);
                // 添加到epoll里面
                std::cout << "9" << std::endl;
                ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev_client);
                if (ret < 0) {
                    perror("epoll_ctl error");
                    return -1;
                }
            } else {
                if (evs[i].events & EPOLLIN) {
                    // 把他摘下来
                    struct epoll_event temp;
                    temp.data.fd = fd;
                    // configureKeepAlive(fd);
                    temp.events = EPOLLIN || EPOLLET;
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &temp);
                    //
                    // pool.enqueue(std::bind(work, fd, std::ref(redis), epoll_fd));
                    pool.submit(work, fd, std::ref(redis), epoll_fd);
                }
            }
        }
    }
    close(epoll_fd);
    return 0;
}

void work(int client_fd, Redis &redis, int epoll_fd) {
    // char buf[1024];
    // int len = recv(client_fd, buf, 1024, 0);
    std::string buf;
    int len = recvMsg(client_fd, buf);
    if (len <= 0) {
        close(client_fd);
    }
    buf[len] = '\0';

    if (len == 0) {
        std::cout << client_fd << "断开连接" << std::endl;
        redis.set_offline(std::to_string(client_fd)); // 设置为离线
        std::cout << "设置离线" << std::endl;
        close(client_fd);
        return;
    } else if (len < 0) {
        // if (errno == EWOULDBLOCK) {
        //     // 心跳超时, 记录日志并断开连接
        //     time_t now = time(NULL);
        //     fprintf(stderr, "[%s] Client disconnected due to heartbeat timeout.\n", ctime(&now));
        //     std::cout << "设置离线" << std::endl;
        //     redis.set_offline(std::to_string(client_fd)); // 设置为离线
        //     close(client_fd);
        // }
        std::cout << "接受失败" << std::endl;
        return;
    }
    //
    // struct timeval timeout;
    // timeout.tv_sec = TIMEOUT_SECONDS;
    // timeout.tv_usec = 0;
    // if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
    //     perror("setsockopt");
    //     close(client_fd);
    //     return;
    // }
    std::cout << "刷新时间" << std::endl;
    std::string str(buf);
    struct epoll_event temp;
    int fd = client_fd;
    int epld = epoll_fd;
    temp.data.fd = fd;
    temp.events = EPOLLIN;
    std::cout << "来自客户端 fd " << client_fd << "  发来消息 " << ": " << str << std::endl;
    // 现在收到传送的消息
    // 处理消息
    std::string ret = fanhui(str, redis, client_fd);
    // printf("返回给客户端 fd %d : %s\n", client_fd, ret.c_str());
    if (ret == "error" || ret == "") {
        // 这里也要挂上去。。。
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &temp);
        return;
    }
    // send(client_fd, ret.c_str(), 1024, 0); // 发送消息
    send_meg(client_fd, ret);
    std::cout << "发送 给 fd" << client_fd << " :" << ret << std::endl;
    // 发给客户端指令
    // 这个
    // 再把树弄上去
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &temp);
    // printf("dsfnsdjklefnsd");
}
// void configureKeepAlive(int curfd) {
//     int keep_alive = 1;     // 开启TCP心跳机制
//     int keep_idle = 1;     // 开始首次心跳监测的TCP空闲时间
//     int keep_interval = 1; // 两次心跳检测的时间
//     int keep_count = 1;     // 连接心跳检测失败次数后，判定连接失效

//     // 开启TCP心跳机制
//     if (setsockopt(curfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keep_alive, sizeof(keep_alive)) == -1) {
//         perror("setsockopt SO_KEEPALIVE");
//         return;
//     }

//     // 设置首次心跳检测前的TCP空闭时间
//     if (setsockopt(curfd, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&keep_idle, sizeof(keep_idle)) == -1) {
//         perror("setsockopt TCP_KEEPIDLE");
//         return;
//     }

//     // 设置两次心跳检测的间隔时间
//     if (setsockopt(curfd, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keep_interval, sizeof(keep_interval)) == -1) {
//         perror("setsockopt TCP_KEEPINTVL");
//         return;
//     }

//     // 设置连续心跳检测失败次数后，判定连接失效
//     if (setsockopt(curfd, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keep_count, sizeof(keep_count)) == -1) {
//         perror("setsockopt TCP_KEEPCNT");
//         return;
//     }

//     std::cout << "TCP Keepalive 已配置" << std::endl;
// }
