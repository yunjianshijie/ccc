#include <iostream>
#include <string>
#include <cstring>
#include <hiredis/hiredis.h>
#include <cstddef>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <future>
#include <time.h>
#include <cstdlib>
#include "../../include/account.hpp"
#include "../../include/back.hpp"
#include <nlohmann/json.hpp>

class Redis {
    // 私密
private:
    redisContext *rc; // 只有一个
public:
    // 构造
    Redis();
    // 析构
    ~Redis() {
        redisFree(rc);
    }
    // redis命令
    // 用户 用哈希表存储
    std::string redis_command(std::string cmd); // redis设置命令

    // 获取新ip
    std::string get_new_id();
    // 设置账号
    std::string setAccount(Account &account);
    // 获取账号
    std::string getAccount(std::string id);
    // 删除账号
    std::string delAccount(std::string &account);
    // 获取账号列表
    std::string getAccountList();
    // json设置账号
    std::string json_setAccount(std::string json);
    // json获取账号
    std::string json_getAccount(std::string id);
    // json删除账号
    std::string json_delAccount(std::string json);
    // json获取账号列表
    std::string json_getAccountList();
    // 判断账号是否存在
    // Account isAccountExist(std::string id );
    // json设置在线
    std::string josn_state_online(std::string json);
    // json设置离线
    std::string state_offline(std::string id);
    // json获取用户信息
    std::string json_getUserInfo(std::string id);
    //
    // 在线fd 对应uid
    // 设置在线
    void set_online(std::string id, std::string fd); //
    // 获取在线
    std::string get_online(std::string id);
    // 设置离线
    void set_offline(std::string fd);
    //
    // // 好友设置，好友申请列表（待处理）
    // void set_friend(std::string id, std::string friend_id);
    // 添加每个人的好友（好友申请列表）
    void apply_friend(std::string id, std::string friend_id, std::string friend_name, std::string msg);
    // 获取好友申请列表
    std::string get_apply_friend(std::string id);
    // 每个人好友列表
    void add_friend_list(std::string id, std::string friend_id);
    // 删除好友申请列表
    void del_apply_friend(std::string id, std::string friend_id);
    // 获取好友列表
    std::string get_friend_list(std::string id);
    // 对于有序list 写入
    void list_write(std::string key, std::string value);
    // 有序list读出
    std::vector<std::string> list_read(std::string key);
    // 有序list删除
    std::string list_del(std::string key);
    // 有序列表获取长度
    int list_len(std::string key);
    //  查询key键是否存在
    std::vector<std::string> key_exist(std::string key);
    // 集合添加
    void add_set(std::string key, std::string value);
    // 集合删除
    void del_set(std::string key, std::string value);
    // 集合查询
    std::vector<std::string> get_set(std::string key);
    // 查询集合元素
    bool is_set(std::string key, std::string value);
    // 获取群id
    std::string get_group_id();
    //哈希表设置
    void set_hash(std::string key, std::string field, std::string value);
    //
    std::string get_hash(std::string key, std::string field);
    std::vector<std::string> find_key(std::string key);
    bool key_bool(std::string key);
};

// 群主：groupowner:
// 群员：groupjoin:
// 群管理： groupmanage: