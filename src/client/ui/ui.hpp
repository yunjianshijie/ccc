#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "../../include/back.hpp"
#include "../../include/account.hpp"
void begin();
void user_long(std::string UerName);
// 开始页面
std::string register_ui();
// 在注册时输入密码
void register_ui1(Account &account);
int login_ui(Account &account);
// 用户主界面
std::string user_main(std::string UerName, std::string UerIp);
// 用户修改密码

// 修改用户名
std::string change_name_ui();
// 添加好友
std::string add_friend_ui();
// 查询用户
std::string user_query_ui();
// 用用户查询
void user_query_ui1(std::string j, std::string uid);
// 处理数字
// int main_t(std::string buf);
// int strToNum(std::string str);
