
#include "read.hpp"

int writen(int fd, std::string msg) {
    // 发送要把头带上
    int count = msg.size();
    char *buf = (char *)msg.c_str();
    while (count > 0) {
        int len = send(fd, buf, count, 0);
        if (len < 0) {
            // close(fd);
            return -1;
        } else if (len == 0) {
            continue;
        }
        count -= len;
        buf += len;
    }
    return msg.size();
}
// util
int send_meg(int fd, std::string msg) {
    if (msg == "" || fd < 0) {
        return -1;
    }
    uint32_t bigLen = htonl(msg.size());
    int size = msg.size();
    std::string data(4 + size, '\0'); // 4个字节+字符串长度
    memcpy(data.data(), &bigLen, 4);
    memcpy(data.data() + 4, msg.data(), size);
    int ret = writen(fd, data);
    return ret;
}

int readn(int fd, int size, char *buf) {
    char *pt = buf;
    int count = size;
    while (count > 0) {
        int len = recv(fd, pt, count, 0);
        if (len == -1) {
            return -1;
        } else if (len == 0) {
            return size - count;
        }
        pt += len;
        count -= len;
    }
    return size; // 返回读取的字节数
}
int recvMsg(int cfd, std::string &msg) {
    // 接收数据
    // 1. 读数据头
    int len = 0;
    readn(cfd, 4, (char *)&len);
    len = ntohl(len);
    //printf("数据块大小: %d\n", len);
    // 根据读出的长度分配内存，+1 -> 这个字节存储\0
    char *buf = (char *)malloc(len + 1);
    int ret = readn(cfd, len, buf);
    if (ret != len) {
        // close(cfd);
        // free(buf);
        return -1;
    }
    if (ret == 0) {
        // printf("客户端关闭连接\n");
        return 0;
    }
    buf[len] = '\0';
    msg = buf;
    return ret;
}

// /*
// 函数描述: 发送指定的字节数
// 函数参数:
//     - fd: 通信的文件描述符(套接字)
//     - msg: 待发送的原始数据
//     - size: 待发送的原始数据的总字节数
// 函数返回值: 函数调用成功返回发送的字节数, 发送失败返回-1
// */
// int writen(int fd, const char *msg, int size) {
//     const char *buf = msg;
//     int count = size;
//     while (count > 0) {
//         int len = send(fd, buf, count, 0);
//         if (len == -1) {
//             close(fd);
//             return -1;
//         } else if (len == 0) {
//             continue;
//         }
//         buf += len;
//         count -= len;
//     }
//     return size;
// }

// /*
// 函数描述: 发送带有数据头的数据包
// 函数参数:
//     - cfd: 通信的文件描述符(套接字)
//     - msg: 待发送的原始数据
//     - len: 待发送的原始数据的总字节数
// 函数返回值: 函数调用成功返回发送的字节数, 发送失败返回-1
// */
// int sendMsg(int cfd, char *msg, int len) {
//     if (msg == NULL || len <= 0 || cfd <= 0) {
//         return -1;
//     }
//     // 申请内存空间: 数据长度 + 包头4字节(存储数据长度)
//     char *data = (char *)malloc(len + 4);
//     int bigLen = htonl(len);
//     memcpy(data, &bigLen, 4);
//     memcpy(data + 4, msg, len);
//     // 发送数据
//     int ret = writen(cfd, data, len + 4);
//     // 释放内存
//     free(data);
//     return ret;
// }

// /*
// 函数描述: 接收指定的字节数
// 函数参数:
//     - fd: 通信的文件描述符(套接字)
//     - buf: 存储待接收数据的内存的起始地址
//     - size: 指定要接收的字节数
// 函数返回值: 函数调用成功返回发送的字节数, 发送失败返回-1
// */
// int readn(int fd, char *buf, int size) {
//     char *pt = buf;
//     int count = size;
//     while (count > 0) {
//         int len = recv(fd, pt, count, 0);
//         if (len == -1) {
//             return -1;
//         } else if (len == 0) {
//             return size - count;
//         }
//         pt += len;
//         count -= len;
//     }
//     return size;
// }

// /*
// 函数描述: 接收带数据头的数据包
// 函数参数:
//     - cfd: 通信的文件描述符(套接字)
//     - msg: 一级指针的地址，函数内部会给这个指针分配内存，用于存储待接收的数据，这块内存需要使用者释放
// 函数返回值: 函数调用成功返回接收的字节数, 发送失败返回-1
// */
// int recvMsg(int cfd, char **msg) {
//     // 接收数据
//     // 1. 读数据头
//     int len = 0;
//     readn(cfd, (char *)&len, 4);
//     len = ntohl(len);
//     printf("数据块大小: %d\n", len);

//     // 根据读出的长度分配内存，+1 -> 这个字节存储\0
//     char *buf = (char *)malloc(len + 1);
//     int ret = readn(cfd, buf, len);
//     if (ret != len) {
//         close(cfd);
//         free(buf);
//         return -1;
//     }
//     buf[len] = '\0';
//     *msg = buf;

//     return ret;
// }