// #include"redis.hpp"
#include "redis.hpp"
#include <sys/socket.h>
#include "../../include/read.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
typedef std::string (*func)(nlohmann::json &j, Redis &redis);
// std::map<int, func> fun_map;
std::string numToStr(int num);
std::string fanhui(std::string str, Redis &redis, int c_fd);
// 登录josn 字符串
std::string josn_str_login(std::string id, Account account);
// 注册josn 字符串
std::string josn_str_register(std::string id);
// 聊天josn 字符串
std::string josn_str_chat(std::string id, std::string name, std::string message);
// 登录josn 返回字符串
std::string josn_str_login_ret(std::string josn, std::string ThePassword, std::string TheId);
//
std::string fun_init(int mode, nlohmann::json &j, Redis &redis);
//
std::string fun_bigin(nlohmann::json &j, Redis &redis);
std::string fun_login(nlohmann::json &j, Redis &redis);
std::string fun_register(nlohmann::json &j, Redis &redis);
std::string fun_login_success(nlohmann::json &j, Redis &redis);
// 处理退出
std::string fun_exit(nlohmann::json &j, Redis &redis);
// 用户查询
std::string fun_user_query(nlohmann::json &j, Redis &redis);
// 添加好友
std::string fun_add_friend(nlohmann::json &j, Redis &redis);
std::string fun_friend_apply_result(nlohmann::json &j, Redis &redis);
// 改名
std::string fun_change_name(nlohmann::json &j, Redis &redis);
// 发送
std::string send_fd(int c_fd, std::string message);
// 接收
std::string fun_apply_friend(nlohmann::json &j, Redis &redis);
// 获取好友列表
std::string fun_friend_list(nlohmann::json &j, Redis &redis);
// 好友聊天
std::string fun_friend_char(nlohmann::json &j, Redis &redis);
// 聊天记录
std::string fun_friend_history(nlohmann::json &j, Redis &redis);
// 屏蔽好友
std::string fun_friend_shield(nlohmann::json &j, Redis &redis);
// 取消屏蔽
std::string fun_friend_unshield(nlohmann::json &j, Redis &redis);
//
std::string fun_exit1(nlohmann::json &j, Redis &redis);
// 删除好友
std::string fun_friend_delete(nlohmann::json &j, Redis &redis);
// 群聊
// 创建群聊
std::string fun_group_create(nlohmann::json &j, Redis &redis);
// 获取群聊列表
std::string fun_group_list(nlohmann::json &j, Redis &redis);
// 获取群聊信息
std::string fun_group_add(nlohmann::json &j, Redis &redis);
// 发送群聊申请
std::string fun_group_add2(nlohmann::json &j, Redis &redis);
// 群聊管理的看
std::string fun_group_manage(nlohmann::json &j, Redis &redis);
// 邀请
std::string fun_group_invite(nlohmann::json &j, Redis &redis);

// 修改密码

std::string fun_change_pass(nlohmann::json &j, Redis &redis);
std::string fun_change_pass1(nlohmann::json &j, Redis &redis);
// 查看群员
std::string fun_group_look(nlohmann::json &j, Redis &redis);
//
std::string fun_group_massage(nlohmann::json &j, Redis &redis);
// 群员离开
std::string fun_group_leave(nlohmann::json &j, Redis &redis);
// 群主踢人
std::string fun_group_kick_o(nlohmann::json &j, Redis &redis);
// 群主离开
std::string fun_group_leave_o(nlohmann::json &j, Redis &redis);
// 设置管理
std::string fun_group_set_ma(nlohmann::json &j, Redis &redis);
// 设置禁言
std::string fun_group_set_ban_o(nlohmann::json &j, Redis &redis);
// 管理员踢人
std::string fun_group_kick(nlohmann::json &j, Redis &redis);

std::string fun_group_set_ban(nlohmann::json &j, Redis &redis);
// 看群聊申请列表
std::string fun_group_will_list(nlohmann::json &j, Redis &redis);
//
std::string fun_group_chuli_will(nlohmann::json &j, Redis &redis);
// 历史记录
std::string fun_group_his(nlohmann::json &j, Redis &redis);
//
bool isJsonString(std::string &str);

std::string fun_file_send(nlohmann::json &j, Redis &redis, int c_fd);

std::string fun_file_receive_g(nlohmann::json &j, Redis &redis);
// 文件
std::string fun_file_receive_g1(nlohmann::json &j, Redis &redis);

//
std::string fun_file_receive_f(nlohmann::json &j, Redis &redis);
std::string fun_file_receive(nlohmann::json &j, Redis &redis, int new_fd);