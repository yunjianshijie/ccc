
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
#include <nlohmann/json.hpp>
#include "ui/ui.hpp"
#include <queue>
#include <thread>
#include <future>
#include "../include/read.hpp"
void pp(std::vector<std::string> vec);
class Socket {
private:
    struct sockaddr_in addr; //
    int just;                // 如果是1，则代表非用户操作, 如果是2，则代表用户操作
    Account account;
    int strToNum(std::string str) {
        std::stringstream ss(str);
        // 如果传回来不是数字就用josn读回来
        int num = 0;
        ss >> num;
        return num;
    }
    std::string ip;
    int hand;
    // std::queue<std::string> messageQueue;
    // std::mutex queueMutex;
    // std::condition_variable queueCondVar;
    // 消息队列
    std::queue<std::string> message_queue;
    // 队列互斥锁
    std::mutex queue_mutex;
    // 条件变量
    std::condition_variable queue_cv;
    // 线程结束标志
    std::mutex mtx;
    std::condition_variable cv;
    bool result_ready = false;
    int state;           // 状态 登录，私聊 群聊
    std::string char_id; // 聊天对象id , 群聊id
    std::string message;
    std::vector<std::string> message_vec;

public:
    int server_fd;
    std::string buf;                 // 服务器传回来的信息
    int client_fd;                   // 客户端ip
    int mode;                        // mode 记录之后的操作
    Socket(int handle, char ip[30]); // handle 是服务器端口号
    Socket(Socket &socket) = delete;
    Socket &operator=(Socket &socket) = delete;
    ~Socket() {
        close(client_fd); // 关闭客户端的ip
    }
    // 处理主要逻辑
    void socket_do();
    // 发送数据
private:
    bool send_string(std::string chuan);
    // 接受数据
    std::string receive_string();
    // 发送注册json数据
    bool send_json_usr_register(Account &account);
    // 接收注册json数据
    std::string receive_json_usr_id();
    // 记住id
    bool remember_id(std::string id);
    // 接收登录json数据
    Account receive_json_usr_login();
    // 发送登录json数据
    std::string send_json_usr_login(Account &account);
    // 退出登录json数据
    std::string send_json_usr_exit();
    // 登录成功发送
    std::string send_josn_login_success();
    // 错误信息
    bool error_string(std::string str);
    // 修改密码
    bool change_pass();
    // 结束在Usr
    // 进入用户界面
    void user_run();
    // 查看自己信息1
    void showSelfInfo(int n);
    // 查询用户2
    void queryUser();
    // 查看好友列表3
    void showFriendList();
    // 查看群聊列表4
    void showGroupList();
    // 添加好友5
    void addFriend();
    // 查看好友申请6
    void apply_FriendList();
    // 查看群聊申请7
    void apply_GroupList(std::string group_id);
    // 添加群聊8
    void addGroup();
    // 创建群聊9
    void createGroup();
    // 查看聊天记录10
    void showChatRecord();
    // 注销账号11
    bool cancel();
    // 好友
    std::string send_json_user_que(std::string id); // 发送添加好友请求
    // 修改昵称
    void send_change_name(std::string name);
    //
    std::string send_json_friend_list();
    // 接受消息
    std::string receive_message();
    std::string receive_message1(int fd);
    // 线程接收消息
    void receive_json();
    //
    int receive_josn_user(std::string massage);

    // 发送
    // 发送好友同样消息
    std::string send_json_friend_apply(std::string id, std::string friend_id, std::string chioce);
    // 屏蔽好友
    std::string send_json_friend_shield(std::string friend_id);
    // 取消屏蔽好友
    std::string send_json_friend_unshield(std::string friend_id);

    // 打印用户信息
    void print_user_qu(std::string massage);
    void print_friend_add(std::string message);
    void print_friend_add_2(std::string message);
    void print_friend_apply_list(std::string message);           // 打印好友申请列表
    void print_friend_apply_list2(std::vector<std::string> vec); // 打印好友申请列表2
    void print_friend_list(std::string message);                 // 打印好友列表
    void print_friend_list2(std::vector<std::string> vec);
    void print_friend_message(std::string message);
    // 私聊
    void private_chat(std::string id, std::string friend_id, std::string friend_name, std::string fd);
    // 发送
    std::string send_josn_chat_frined(std::string id, std::string friend_id, std::string msg, std::string fd, std::string send_id);

    void print_friend_history_message(std::string message);
    void print_friend_history_message2(std::vector<std::string> vec);
    void print_login_success(std::string message);
    void print_friend_shield(std::string message);
    void print_friend_unshield(std::string message);
    void print_friend_shielded(std::string message);
    void print_friend_dleed(std::string message);
    void print_group_create(std::string message);
    void print_group_list(std::string message);
    // 群聊
    void group_do(std::string id, std::string group_id, std::string group_name, int rank);
    //
    void group_chat(std::string id, std::string group_id, std::string group_name, std::string fd);
    // 发送
    // GROUP_MESSAGE
    void group_message(std::string id, std::string group_id, int rank);
    // 添加群聊
    void print_group_add(std::string message);
    // 查看群聊申请
    void print_group_add2(std::string message);
    // 获得了群管理
    void print_group_manage(std::string message);
    //
    void print_group_look(std::string message);
    //
    void owner_ma(std::string id, std::string group_id);
    void chengyuan_ma(std::string id, std::string group_id);
    void print_group_message(std::string message);
    void inv(std::string id, std::string group_id);
    void chankan_num(std::string id, std::string group_id);

    //
    void un_owner_quik(std::string id, std::string group_id);
    // 打印剔除借过
    void print_group_kick_o(std::string message);
    // 群主退群（解散了）
    void un_owner_quik_o(std::string id, std::string group_id);
    // 设置管理
    void set_ma(std::string id, std::string group_id);
    void print_group_set_ma(std::string message);
    // 设置禁言
    void set_ban_o(std::string id, std::string group_id);
    void print_group_set_ban_o(std::string message);

    // 管理管理
    void ma_ma(std::string id, std::string group_id);
    // 管理禁言
    void set_ban(std::string id, std::string group_id);
    void print_group_set_ban(std::string message);
    //
    // ka
    void print_group_will_list(std::string message);
    // 历史记录
    void print_group_his(std::string message);
    void print_group_his2(std::string message);
    // 判断是否群还在

    // 文件
    void file_do();
    void file_send();
    void file_send_group();
    void file_send_friend();
    void send_now(std::string path, std::string will, std::string will_id);
    void print_file_send(std::string message);
    void file_receive();
    void file_receive_group();
    void file_receive_friend();
    void print_file_receive_g(std::string message);
    void print_file_receive_g1(std::string message);
    void print_file_receive_f(std::string message);

    bool send_string1(std::string chuan, int fd);

    void getget_file(std::string name);
    //
    void print_group_invite_q(std::string message);

    void print_friend_apply_q(std::string message);

};
