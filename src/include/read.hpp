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
#include <queue>
#include <thread>
#include <future>
// std::string getMeg();
// std::string getMeg(int fd);
int writen(int fd, std::string meg);
int send_meg(int fd, std::string msg);
int readn(int fd, int size, char *buf); // 读取指定长度的数据
int recvMsg(int cfd, std::string &msg);

//
