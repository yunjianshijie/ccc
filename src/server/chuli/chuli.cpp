#include "chuli.hpp"
#include <filesystem>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

// using json = nlohmann::json;
std::map<int, func> fun_map{
    {0, fun_bigin},
    {LOGIN, fun_login},
    {REGISTER, fun_register},
    {LOGIN_SUCCESS, fun_login_success},
    {USER_QUERY, fun_user_query},
    {FRIEND_ADD, fun_add_friend},
    {FRIEND_APPLY_LIST, fun_apply_friend},
    {FRIEND_APPLY, fun_friend_apply_result},
    {CHANGE_NAME, fun_change_name},
    {FRIEND_LIST, fun_friend_list},
    {FRIEND_MESSAGE, fun_friend_char},
    {HISTORY_MESSAGE, fun_friend_history},
    {FRIEND_SHIELD, fun_friend_shield},
    {FRIEND_UNSHIELD, fun_friend_unshield},
    {EXIT, fun_exit1},
    {FRIEND_DELETE, fun_friend_delete},
    {GROUP_CREATE, fun_group_create},
    {GROUP_LIST, fun_group_list},
    {GROUP_ADD, fun_group_add},
    {GROUP_ADD2, fun_group_add2},
    {GROUP_MANAGE, fun_group_manage},
    {CHANGE_PASS, fun_change_pass},
    {CHANGE_PASS1, fun_change_pass1},
    {GROUP_INVITE, fun_group_invite},
    {GROUP_LOOK_NUM, fun_group_look},
    {GROUP_MESSAGE, fun_group_massage},
    {GROUP_LEAVE, fun_group_leave},
    {GROUP_KICK_O, fun_group_kick_o},
    {GROUP_LEAVE_O, fun_group_leave_o},
    {GROUP_SET_MA, fun_group_set_ma},
    {GROUP_SET_BAN_O, fun_group_set_ban_o},
    {GROUP_KICK, fun_group_kick},
    {GROUP_SET_BAN, fun_group_set_ban},
    {GROUP_WILL_LIST, fun_group_will_list},
    {GROUP_CHULI_WILL, fun_group_chuli_will},
    {GROUP_HIS, fun_group_his},
    // {FILE_RECEIVE, fun_file_receive},
    {FILE_RECEIVE_G, fun_file_receive_g},
    {FILE_RECEIVE_G1, fun_file_receive_g1},
    {FILE_RECEIVE_F, fun_file_receive_f},
};

// 这里是处理主函数
// 这里默认buffer为 josn
// 返回josn
std::string fanhui(std::string str, Redis &redis, int c_fd) {
    // std::cout << "buffer:" << buffer << std::endl;
    // std::string str(buffer);
    // std::cout << "str:" << str << std::endl;
    nlohmann::json j = nlohmann::json::parse(str);
    // 创建类型
    // 解析

    int mode = j["mode"];
    if (mode == FILE_SEND) {
        return fun_file_send(j, redis, c_fd);
    }
    if (mode == FILE_RECEIVE) {
        return fun_file_receive(j, redis, c_fd);
    }
    if (mode == 0) {
        j["c_fd"] = c_fd;
    }
    std::cout << "mode:" << mode << std::endl;
    return fun_init(mode, j, redis);
}

// 定义一个函数类型
typedef std::string (*func)(nlohmann::json &j, Redis &redis);
std::string fun_init(int mode, nlohmann::json &j, Redis &redis) {
    auto it = fun_map.find(mode);
    if (it != fun_map.end()) {
        return it->second(j, redis);
    } else {
        return "error";
    }
}

// 这里是处理josn
std::string fun_bigin(nlohmann::json &j, Redis &redis) {
    // std::string h = numToStr(0);
    nlohmann::json j1;
    j1["mode"] = 0;
    j1["c_fd"] = j["c_fd"];
    return j1.dump();
}

std::string fun_login(nlohmann::json &j, Redis &redis) {
    std::string h = numToStr(1);
    // h=1
    std::string TheId = j["id"];
    std::string ThePassword = j["password"];
    std::string jo = redis.getAccount(TheId);
    std::cout << "jo:" << jo << std::endl;
    return josn_str_login_ret(jo, ThePassword, TheId);
}

std::string fun_register(nlohmann::json &j, Redis &redis) {
    std::string h = numToStr(SUCCESS);
    // 将数据传入数据库
    Account account(j["name"], j["password"], j["question"], j["answer"]);
    // g o账号
    std::string id = redis.setAccount(account);
    if (id == "") {
        return "error";
    }
    return josn_str_register(id);
}
// 登录成功
std::string fun_login_success(nlohmann::json &j, Redis &redis) {
    std::string id = j["id"];
    int fd = j["fd"];
    // 将哈希  fd（-1） 改成 fd
    // 退出后再改成 -1
    std::string com = "HSET account:" + id + " fd " + numToStr(fd);
    // 写进redis
    std::cout << "登录成功" << std::endl;
    redis.set_online(id, numToStr(fd));
    std::cout << "com:" << com << std::endl;
    redis.redis_command(com);
    // 这里要返回s消息
    // 查询id_s_前缀的队列
    std::string key = id + "_s_*";
    std::cout << key << std::endl;
    std::vector<std::string> vec = redis.key_exist(key);
    std::vector<std::string> mas = redis.get_set("ac_msg:" + id);
    nlohmann::json j1;
    j1["mode"] = LOGIN_SUCCESS;
    j1["id"] = id;
    j1["message"] = vec;
    j1["ac_msg"] = mas;
    //
    // redis.list_del("ac_msg:" + id);
    std::cout << "j1:" << j1.dump() << std::endl;
    send_fd(fd, j1.dump());
    return "";
}

std::string fun_change_name(nlohmann::json &j, Redis &redis) {
    std::string id = j["id"];
    std::string name = j["name"];
    std::string com = "HSET account:" + id + " name " + name;
    redis.redis_command(com);
    return "";
}

std::string josn_str_login(std::string y_n, Account account) { // 登录
    nlohmann::json j;
    std::cout << "dsfjdsklfsd" << std::endl;
    j["mode"] = LOGIN;
    // 处理是否有这个账号
    j["y_n"] = y_n;
    j["id"] = account.id;
    j["name"] = account.name;
    j["question"] = account.question;
    j["answer"] = account.get_answer();
    j["password"] = account.get_password();
    std::cout << "j:" << j.dump() << std::endl;
    return j.dump();
}
std::string josn_str_register(std::string id) {
    nlohmann::json j;
    j["mode"] = REGISTER;
    j["id"] = id;
    return j.dump();
}

std::string josn_str_chat(std::string id, std::string name, std::string message) {
    nlohmann::json j;
    // j["mode"] = CHAT;
    j["id"] = id;
    j["name"] = name;
    j["message"] = message;
    return j.dump();
}

std::string numToStr(int num) {
    return std::to_string(num);
}

std::string josn_str_login_ret(std::string josn, std::string ThePassword, std::string TheId) {
    std::string id;
    if (josn == "null") {
        printf("没有此账号\n");
        id = "-2";
        return josn_str_login("no", Account("", "", "", "", id));
    }
    nlohmann::json j1 = nlohmann::json::parse(josn);
    // std::cout << "j1:" << j1 << std::endl;
    printf("有此账号\n");
    std::string name = j1["name"];
    std::string anwser = j1["answer"];
    std::string password = j1["password"];
    std::string question = j1["question"];
    std::string fd = j1["fd"];
    id = TheId;
    if (fd != "-1") {
        std::cout << "该账号已登录" << std::endl;
        return josn_str_login("no", Account("", "", "", "", "-3"));
    }
    Account account(name, password, question, anwser, id);

    if (password != ThePassword) {
        std::cout << "密码错误" << std::endl;
        return josn_str_login("no", Account("", "", "", "", "-1"));
    }
    return josn_str_login("yes", account);
}

std::string fun_exit(nlohmann::json &j, Redis &redis) {
    return "";
    std::string id = j["id"];
    return redis.json_getUserInfo(id);
}

std::string fun_user_query(nlohmann::json &j, Redis &redis) {
    std::string id = j["id"];
    // id的格式为 account:id
    return redis.json_getUserInfo(id);
}
std::string fun_add_friend(nlohmann::json &j, Redis &redis) {
    printf("添加好友\n");
    std::string id = j["id"];
    std::string friend_id = j["friend_id"];
    std::string msg = j["msg"];
    std::string name = j["name"];
    std::string in = redis.json_getUserInfo(friend_id);
    nlohmann::json j1 = nlohmann::json::parse(in); // 查询好友信息
    std::cout << "in:" << in << std::endl;
    j1["mode"] = FRIEND_ADD;
    if (j1["fd"] == "-2") {
        printf("没有此账号\n");
    } else if (j1["fd"] == "-1") {
        std::cout << "好友不在线" << std::endl;
        // 先将它存入redis
        redis.apply_friend(friend_id, id, name, msg);
        nlohmann::json js;
        js["mode"] = "f_add";
        js["id"] = id;
        js["name"] = name;
        redis.add_set("ac_msg:" + id, js.dump());
        // 等待好友上线
    } else {
        std::cout << "好友在线" << std::endl;
        // 发送给好友
        // 好友的fd
        std::string fd1 = j1["fd"];
        std::cout << "size:" << in.size() << std::endl;
        // send(std::stoi(fd1), j1.dump().c_str(), j1.dump().size(), 0);
        nlohmann::json j2;
        j2["mode"] = FRIEND_ADD_2;
        j2["id"] = id;
        j2["msg"] = msg;
        j2["friend_id"] = friend_id;
        j2["name"] = name;
        send_fd(std::stoi(fd1), j2.dump());
        // 给好友的好友申请列表加上
        redis.apply_friend(friend_id, id, name, msg);
        return j1.dump();
    }
    // 先查用户存不存在
    // 存在后 用户在不在线
    // 在线则发送消息
    // 不在线则存入redis
    //    return redis.json_addFriend(id, friend_id);
    return j1.dump();
}
std::string fun_friend_apply_result(nlohmann::json &j, Redis &redis) {
    try {
        std::string id = j["id"];
        std::string friend_id = j["friend_id"];
        std::string chioce = j["chioce"];
        if (redis.is_set("friend:" + id, friend_id)) {
            // 发送你已经被加好友了
            // 删申请了
            redis.del_apply_friend(id, friend_id);
            nlohmann::json j1;
            j1["mode"] = FRIEND_SHIELDED;
            j1["or"] = "4";
            return j1.dump();
        }
        nlohmann::json json1;
        json1["mode"] = FRIEND_APPLY_Q;
        json1["name"] = redis.get_hash("account:" + friend_id, "name");
        json1["id"] = id;
        json1["chioce"] = chioce;
        std::string fd = redis.get_hash("account:" + friend_id, "fd");
        send_fd(std::stoi(fd), json1.dump());
        if (chioce == "1") {
            // 加好友
            redis.add_friend_list(id, friend_id);
            redis.add_friend_list(friend_id, id);
        }
        // 删申请
        redis.del_apply_friend(id, friend_id);
        return "";

    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
    return "";
}

// 查看好友申请列表
std::string fun_apply_friend(nlohmann::json &j, Redis &redis) {
    std::string id = j["id"];
    return redis.get_apply_friend(id);
}
//
std::string fun_friend_list(nlohmann::json &j, Redis &redis) {
    std::string id = j["id"];
    return redis.get_friend_list(id);
}
//
std::string fun_friend_char(nlohmann::json &j, Redis &redis) {
    std::string id = j["id"];
    std::string friend_id = j["friend_id"];
    std::string msg = j["msg"];
    std::string fd = j["f_fd"];
    std::string send_id = j["send_id"]; // 发送者id
    std::time_t now = std::time(nullptr);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
    std::string time_str = ss.str();
    nlohmann::json j1;
    j1["send_id"] = send_id;
    j1["id"] = friend_id;
    j1["friend_id"] = id;
    j1["msg"] = msg;
    j1["time"] = time_str;
    if (!redis.is_set("friend:" + id, friend_id)) {
        // 如果已经不是好友了那就只删聊天记录了
        nlohmann::json j1;
        j1["mode"] = FRIEND_DLEED;
        return j1.dump();
    };
    std::string ret = redis.getAccount(friend_id);
    std::cout << "ret:" << ret << std::endl;
    nlohmann::json j2 = nlohmann::json::parse(ret);

    std::string fd1 = j2["fd"];
    std::string key4 = "shield:" + friend_id;
    // 判断屏蔽
    nlohmann::json j3;
    j3["mode"] = FRIEND_SHIELDED;
    // std::cout << "11111111" << std::endl;
    if (redis.is_set(key4, id)) {
        j3["or"] = "1";
        return j3.dump(); // 被屏蔽了
    }
    std::cout << "11111111" << std::endl;
    std::string key1 = "shield:" + id;
    printf("key1:%s\n", key1.c_str());
    if (redis.is_set(key1, friend_id)) {
        j3["or"] = "2";
        return j3.dump(); // 屏蔽了他
    }
    //
    //
    std::cout << "11111111" << std::endl;
    {
        std::string key = "frind:" + friend_id;
        if (redis.is_set(key, id)) {
            j3["or"] = "3";
            return j3.dump();
        }
    }
    if (fd1 != "-1") {
        // 给好友发送东西
        j1["mode"] = FRIEND_MESSAGE;
        // j1["fd"] = fd;
        // 这里应该时实获得fd，下线就是-1
        std::cout << "发送给 fd:" << fd1 << j1.dump() << std::endl;
        send_fd(std::stoi(fd1), j1.dump());
    } else {
        // 写进离线消息
        j1["mode"] = OFFLINE_MESSAGE;
        // std::string key = id + "_s_" + friend_id; // 私聊
        std::string key1 = friend_id + "_s_" + id;
        // redis.list_write(key, j1.dump());
        redis.list_write(key1, j1.dump());
        // 用有序列表加两个
    }
    j1["mode"] = HISTORY_MESSAGE;
    // redis.add_chat(id, friend_id, msg);
    std::string key2 = id + "_l_" + friend_id; // 私聊历史记录
    std::string key3 = friend_id + "_l_" + id; // 私聊历史记录
    redis.list_write(key3, j1.dump());
    redis.list_write(key2, j1.dump());
    return ""; // 不发给 id客户端发送东西
}
std::string fun_friend_history(nlohmann::json &j, Redis &redis) {
    std::string id = j["id"];
    std::string friend_id = j["friend_id"];
    std::string key = id + "_l_" + friend_id;
    std::vector<std::string> v = redis.list_read(key);
    // 读完把redis删掉
    std::string key1 = id + "_s_" + friend_id;
    std::cout << "key1:" << key1 << std::endl;
    redis.list_del(key1);
    nlohmann::json j1;
    j1["mode"] = HISTORY_MESSAGE;
    j1["history"] = v;
    std::cout << j1.dump() << std::endl;
    return j1.dump();
}

std::string send_fd(int c_fd, std::string message) {
    try {
        // int message_size = message.size();
        // std::cout << "message_size:" << message_size << std::endl;
        // std::cout << "message:" << message << std::endl;
        // int ret = send(c_fd, message.c_str(), 1024, 0);
        // std::cout << "send:" << ret << std::endl;
        int len = send_meg(c_fd, message);
        if (len == -1) {
            perror("send");
            return "error"; // 发送失败
        } else if (len == 0) {
            return "error";
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
    return "";
}

std::string fun_friend_shield(nlohmann::json &j, Redis &redis) {
    // 设置shield:id 的集合
    std::string id = j["id"];
    std::string friend_id = j["friend_id"];
    std::string key = "shield:" + id;

    nlohmann::json j1;
    j1["mode"] = FRIEND_SHIELD;
    if (redis.is_set(key, friend_id)) {
        j1["y_n"] = "no";
    } else {
        j1["y_n"] = "yes";
        redis.add_set(key, friend_id);
    }
    return j1.dump();
}
std::string fun_friend_unshield(nlohmann::json &j, Redis &redis) {
    // 删除shield:id 的集合
    std::string id = j["id"];
    std::string friend_id = j["friend_id"];
    std::string key = "shield:" + id;
    nlohmann::json j1;
    j1["mode"] = FRIEND_UNSHIELD;
    if (redis.is_set(key, friend_id)) {
        j1["y_n"] = "yes";
        redis.del_set(key, friend_id);
    } else {
        j1["y_n"] = "no";
    }
    return j1.dump();
}
std::string fun_exit1(nlohmann::json &j, Redis &redis) {
    nlohmann::json j1;
    j1["mode"] = EXIT;
    std::string ret = redis.getAccount(j["id"]);
    nlohmann::json json = nlohmann::json::parse(ret);
    std::string fd = json["fd"];
    redis.set_offline(fd); // 设置为离线
    return j1.dump();
}
std::string fun_friend_delete(nlohmann::json &j, Redis &redis) {
    // 删掉数据库里面有关他的好友和屏蔽消息，和历史记录
    std::string id = j["id"];
    std::string friend_id = j["friend_id"];
    if (!redis.is_set("friend:" + id, friend_id)) {
        // 如果已经不是好友了那就只删聊天记录了
        nlohmann::json j1;
        j1["mode"] = FRIEND_DLEED;
        return j1.dump();
        std::string key1 = id + "_l_" + friend_id;
        redis.list_del(key1);
        std::string key2 = friend_id + "_l_" + id;
        redis.list_del(key2);
        std::string key3 = id + "_s_" + friend_id;
        redis.list_del(key3);
        std::string key4 = id + "_s_" + friend_id;
        redis.list_del(key4);
    };
    std::string key5 = id + "_l_" + friend_id;
    redis.list_del(key5);
    std::string key6 = friend_id + "_l_" + id;
    redis.list_del(key6);
    std::string key7 = id + "_s_" + friend_id;
    redis.list_del(key7);
    std::string key8 = id + "_s_" + friend_id;
    redis.list_del(key8);
    std::string key1 = "friend:" + id;
    redis.del_set(key1, friend_id);
    std::string key2 = "friend:" + friend_id;
    redis.del_set(key2, id);
    std::string key3 = "shield:" + id;
    redis.del_set(key3, friend_id);
    std::string key4 = "shield:" + friend_id;
    redis.del_set(key4, id);
    return "";
    // j["mode"] = FRIEND_DELETE;
    // j["id"] = account.id;
    // j["friend_id"] = id;
}

std::string fun_group_create(nlohmann::json &j, Redis &redis) {
    // 创建群聊
    // 获得id
    std::string group_id = redis.get_group_id();
    std::string group_name = j["name"];
    std::string group_owner = j["id"];
    // 将id放进redis
    redis.add_set("group_ids", group_id);
    std::string key = "group:" + group_id;
    redis.set_hash(key, "group_name", group_name);
    redis.set_hash(key, "group_owner", group_owner);
    // redis.set_hash(key, "group_members", "");
    // 对于个人管理群聊，和群聊群主
    // 用 集合
    key = "groupowner:" + group_owner;
    redis.add_set(key, group_id);
    // 加入的群聊
    key = "groupjoin:" + group_owner;
    redis.add_set(key, group_id);
    nlohmann::json j1;
    j1["mode"] = GROUP_CREATE;
    j1["group_id"] = group_id;
    redis.add_set("group_number:" + group_id, "!" + group_owner);
    return j1.dump();

    // 群聊用哈希表存储
    // 群聊id
    // 群聊名称
    // 群聊群主id !
    // 群聊成员id &//可空-> 现在是空的
    // 管理员id   *//可空-> 现在是空的
    // 申请人员是 %
}

std::string fun_group_list(nlohmann::json &j, Redis &redis) {
    //    // 获得id
    std::string id = j["id"];
    std::string key = "groupjoin:" + id;
    std::vector<std::string> group_ids = redis.get_set(key);
    nlohmann::json j1;
    // 写个集合把id放前面，名字放后面
    for (auto &group_id : group_ids) {
        std::string key1 = "group:" + group_id;
        std::string group_name = redis.get_hash(key1, "group_name");
        group_id += group_name;
    }
    j1["mode"] = GROUP_LIST;
    j1["group_ids"] = group_ids;
    return j1.dump();
}
std::string fun_group_add(nlohmann::json &j, Redis &redis) {
    // 这里是，
    std::string id_name = j["id_name"];
    std::string id = j["id"];
    // 所以来返回 对应的群聊
    std::string key = "group:";
    std::string key1 = "group:" + id_name;
    std::vector<std::string> ret = redis.find_key(key);
    std::vector<std::string> group_ids;
    // std::cout << "ds222afsd" << std::endl;
    for (auto &i : ret) {
        // std::string temp = i.substr(6, -1) + redis.get_hash(i, "name");

        std::string temp1 = i.substr(6, -1);

        std::string temp2 = redis.get_hash(i, "group_name");
        std::cout << temp2 << "_______" << std::endl;
        if (temp1 == id_name || temp2 == id_name) {
            group_ids.push_back(temp1 + temp2);
        }
    }
    nlohmann::json j1;
    j1["mode"] = GROUP_ADD;
    j1["group_ids"] = group_ids;
    return j1.dump();
}
std::string fun_group_add2(nlohmann::json &j, Redis &redis) {
    std::string group_id = j["group_id"];
    std::string id = j["id"];
    //
    if (redis.is_set("group_number:" + group_id, "!" + id) || redis.is_set("group_number:" + group_id, "&" + id)) {
        j["y_n"] = "no";
        return j.dump();
    }
    if (redis.is_set("group_number:" + group_id, "*" + id)) {
        j["y_n"] = "no";
        return j.dump();
    }
    if (redis.is_set("group_will:" + group_id, id)) {
        j["y_n"] = "no1";
        return j.dump();
    }
    j["y_n"] = "yes";
    std::string key = "group:" + group_id;
    // 发送给群主，（管理员）
    // 然后将申请人
    std::string group_owner = redis.get_hash(key, "group_owner");
    // 在这个存来
    redis.add_set("group_will:" + group_id, id);
    // redis.add_set("group_number:" + group_id, "%" + id);
    std::string fd = redis.get_hash("account:" + group_owner, "fd");
    if (fd == "-1") {
        std::cout << "群主不在线" << std::endl;
        // 如果不在线就把这个存
        return j.dump();
    }
    //
    nlohmann::json j1;
    j1["mode"] = GROUP_ADD2;
    j1["group_id"] = group_id;
    j1["id"] = id;

    send_fd(std::atoi(fd.c_str()), j1.dump());
    // 发送给群主，（管理员）
    //
    std::cout << "发送给群主  " << group_owner << " fd " << fd << std::endl;
    return j.dump();
}
//
std::string fun_group_manage(nlohmann::json &j, Redis &redis) {
    // 查询群里面他是什么
    std::string id = j["id"];
    std::string group = j["group"];
    std::string group_id = group.substr(0, 8);
    nlohmann::json json;
    json["group"] = group;
    json["id"] = id;
    json["mode"] = j["mode"]; // 38
    if (redis.is_set("groupowner:" + id, group_id)) {
        //
        json["rank"] = OWNER;
    } else if (redis.is_set("groupmaer:" + id, group_id)) {
        json["rank"] = MANAGER;
    } else if (redis.is_set("groupjoin:" + id, group_id)) {
        json["rank"] = MEMBER;
    } else
        json["rank"] = -1;
    return json.dump();
}

std::string fun_change_pass(nlohmann::json &j, Redis &redis) {
    std::string id = j["id"];
    // 发送答案和密码，问题
    std::string ret = redis.getAccount(id);
    nlohmann::json j2 = nlohmann::json::parse(ret);
    j2["mode"] = CHANGE_PASS;
    return j2.dump();
}
std::string fun_change_pass1(nlohmann::json &j, Redis &redis) {
    std::string pass = j["password"];
    std::string id = j["id"];
    redis.set_hash("account:" + id, "password", pass);
    return "";
}

std::string fun_group_invite(nlohmann::json &j, Redis &redis) {
    // 邀请的人
    std::string id = j["id"];
    // 群
    std::string group_id = j["group"];
    // 查查是不是这个群的人
    std::string my_id = j["my_id"];
    nlohmann::json json;
    json["mode"] = GROUP_INVITE;
    if (redis.is_set("groupjoin:" + id, group_id)) {
        json["y_n"] = "no1";
        return json.dump();
    }
    //  // 查查是不是自己好友
    if (!redis.is_set("friend:" + my_id, id)) {
        json["y_n"] = "no2";
        return json.dump();
    }
    if (!redis.is_set("group_ids", group_id)) {
        json["y_n"] = "no3";
        return json.dump();
    }
    // 如果都不是直接拉进来
    json["y_n"] = "yes";
    redis.add_set("groupjoin:" + id, group_id);
    // 要把它放进群里
    redis.add_set("group_number:" + group_id, "&" + id);
    nlohmann::json json1;
    json1["mode"] = GROUP_INVITE_Q;
    json1["group_id"] = group_id;
    json1["name"] = redis.get_hash("account:" + my_id, "name");
    json1["id"] = my_id;
    std::string fd = redis.get_hash("account:" + id, "fd");
    if (fd != "-1") {
        int fd1 = std::stoi(fd);
        send_fd(fd1, json1.dump());
    }
    // 放进群里面的group_number:里面 ，群主前面是！ 管理员前面是 * 群员前面是 &
    // 在群的hash里面也设置
    // redis.set_hash()
    return json.dump();
}
std::string fun_group_look(nlohmann::json &j, Redis &redis) {
    std::string group_id = j["group_id"];
    nlohmann::json ret;
    ret["mode"] = GROUP_LOOK_NUM;
    std::vector<std::string> num = redis.get_set("group_number:" + group_id);
    ret["num"] = num;
    std::vector<std::string> names;
    for (int i = 0; i < num.size(); i++) {
        names.push_back(redis.get_hash("account:" + num[i].substr(1, -1), "name"));
    }
    ret["names"] = names;
    return ret.dump();
}

std::string fun_group_massage(nlohmann::json &j, Redis &redis) {
    std::string msg = j["msg"];
    std::string id = j["id"];
    std::string group_id = j["group_id"];
    std::string time = j["time"];
    nlohmann::json json;
    json["msg"] = msg;
    json["id"] = id;
    json["time"] = time;
    json["mode"] = GROUP_MESSAGE;
    json["group_id"] = group_id;
    json["name"] = redis.get_hash("account:" + id, "name");
    // 判断是什么身份
    {
        if (redis.is_set("group_ban:" + group_id, id)) {
            json["id"] = "-1";
            return json.dump();
        }
        // 有没有被禁言，被禁言了就返回给自己
    }
    // 判断群聊还在不在
    if (!redis.is_set("group_ids", group_id)) {
        json["id"] = "-5";
        return json.dump();
    }
    try {
        std::vector<std::string> ids = redis.get_set("group_number:" + group_id);
        for (int i = 0; i < ids.size(); i++) {
            if (ids[i].substr(1, -1) == id) {
                if (ids[i][0] == '!') {
                    json["rank"] = OWNER;
                } else if (ids[i][0] == '*') {
                    json["rank"] = MANAGER;
                } else {
                    json["rank"] = MEMBER;
                }
                break; // jjj
            }
        }

        redis.list_write("group_msg:" + group_id, json.dump());
        // 发给每一个群员：%
        for (int i = 0; i < ids.size(); i++) {
            if (ids[i][0] != '%') {
                std::string fd = redis.get_hash("account:" + ids[i].substr(1, -1), "fd");
                // 存进数据库里面
                if (ids[i].substr(1, -1) == id) {
                    // std::cout << "这是自己" << std::endl;
                    continue;
                }
                // std::cout << "id:"<< ids[i].substr(1, -1) << std::endl;
                if (fd != "-1") {
                    std::cout << "将消息发送个给" << fd << "   id  " << ids[i].substr(1, -1) << std::endl;
                    std::cout << json.dump() << std::endl;
                    send_fd(std::stoi(fd), json.dump());
                } else {
                    // 存进数据库里面，上线通知
                }
            }
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
    return "";
}
// 管理员和群聊
std::string fun_group_leave(nlohmann::json &j, Redis &redis) {
    std::string group_id = j["group_id"];
    // nlohmann::json json;
    // json["id"] = "0";
    // json["mode"] = j["mode"];
    // if (!redis.is_set("group_ids", group_id)) {
    //     json["id"] = "-5";
    //     return json.dump();
    // }
    std::string id = j["id"];
    // 删掉群成员列表里面

    std::cout << "DEL:  group_number:" + group_id << "  &" << id << std::endl;
    redis.del_set("group_number:" + group_id, "&" + id);
    redis.del_set("group_number:" + group_id, "*" + id);
    // 删掉自己的群里面
    std::cout << "DEL:  groupjoin:" + id << "  " << group_id << std::endl;
    redis.del_set("groupjoin:" + id, group_id);
    return "";
}

std::string fun_group_kick_o(nlohmann::json &j, Redis &redis) {
    // 查看这个人是不是群里
    nlohmann::json json;
    std::string group = j["group"];
    std::string group_id = group.substr(0, 8);
    json["id"] = "0";

    std::string id = j["id"];

    json["mode"] = j["mode"];
    if (redis.is_set("group_number:" + group_id, "&" + id) || redis.is_set("group_number:" + group_id, "*" + id)) {
        json["y_n"] = "yes";
        redis.del_set("group_number:" + group_id, "&" + id);
        redis.del_set("group_number:" + group_id, "*" + id);
        // 删掉自己的群里面
        redis.del_set("groupmaer:" + id, group_id);
        redis.del_set("groupjoin:" + id, group_id);
        // 还要把禁言的给删了
        redis.del_set("group_ban:" + group_id, id);
    } else {
        json["y_n"] = "no";
    }
    return json.dump();
}

std::string fun_group_leave_o(nlohmann::json &j, Redis &redis) {
    std::string group_id = j["group_id"];
    // 将群解散
    std::string id = j["id"];
    redis.del_set("groupowner:" + id, group_id);
    std::vector<std::string> ids = redis.get_set("group_number:" + group_id);
    for (int i = 0; i < ids.size(); i++) {
        redis.del_set("groupjoin:" + ids[i].substr(1, -1), group_id);
        if (ids[i].substr(0, 1) == "*") {
            // 管理员
            redis.del_set("groupmaer:" + ids[i].substr(1, -1), group_id);
        }
    }
    redis.del_set("group_ids", group_id);
    redis.list_del("group_msg:" + group_id);
    redis.list_del("group_number:" + group_id);
    redis.list_del("group:" + group_id);
    redis.list_del("group_ban:" + group_id);
    return "";
}
std::string fun_group_set_ma(nlohmann::json &j, Redis &redis) {
    std::string id = j["id"];
    std::string group_id = j["group_id"];
    // id是管理
    std::vector<std::string> ids = redis.get_set("group_number:" + group_id);
    nlohmann::json json;
    json["mode"] = j["mode"];

    for (int i = 0; i < ids.size(); i++) {
        if (ids[i].substr(1, -1) == id) {
            // 管理员&& ids[i].substr(0, 1) != '*'
            if (ids[i][0] != '*') {
                // 不是管理员
                redis.add_set("groupmaer:" + ids[i].substr(1, -1), group_id);
                redis.del_set("group_number:" + group_id, "&" + id);
                redis.add_set("group_number:" + group_id, "*" + id);
                json["y_n"] = "yes1";
                return json.dump();
            } else {
                redis.del_set("groupmaer:" + ids[i].substr(1, -1), group_id);
                redis.del_set("group_number:" + group_id, "*" + id);
                redis.add_set("group_number:" + group_id, "&" + id);
                json["y_n"] = "yes2";
                return json.dump();
            }
        }
    }
    json["y_n"] = "no";
    return json.dump();
}

std::string fun_group_set_ban_o(nlohmann::json &j, Redis &redis) {
    std::string id = j["id"];
    std::string group_id = j["group_id"];
    std::vector<std::string> ids = redis.get_set("group_number:" + group_id);
    nlohmann::json json;
    json["mode"] = j["mode"];
    for (int i = 0; i < ids.size(); i++) {
        if (ids[i].substr(1, -1) == id) {
            // 管理员&& ids[i].substr(0, 1) != '*'
            if (!redis.is_set("group_ban:" + group_id, id)) {
                redis.add_set("group_ban:" + group_id, id);
                json["y_n"] = "yse1";
                return json.dump();
            } else {
                redis.del_set("group_ban:" + group_id, id);
                json["y_n"] = "yse2";
                return json.dump();
            }
        }
    }
    json["y_n"] = "no";
    return json.dump();
}

std::string fun_group_kick(nlohmann::json &j, Redis &redis) {
    std::string id = j["id"];
    std::string group = j["group"];
    std::string group_id = group.substr(0, 8);
    std::vector<std::string> ids = redis.get_set("group_number:" + group_id);
    nlohmann::json json;
    json["mode"] = j["mode"];
    for (int i = 0; i < ids.size(); i++) {
        if (ids[i].substr(1, -1) == id) {
            if (ids[i].substr(0, 1) == "!" || ids[i].substr(0, 1) == "*") {
                json["y_n"] = "no1";
                return json.dump();
            }
            json["y_n"] = "yes";
            redis.del_set("group_number:" + group_id, "&" + id);
            redis.del_set("groupjoin:" + id, group_id);
            redis.del_set("group_ban:" + group_id, id);
            return json.dump();
        }
    }
    json["y_n"] = "no";
    return json.dump();
}
std::string fun_group_set_ban(nlohmann::json &j, Redis &redis) {
    std::string id = j["id"];
    std::string group = j["group"];
    std::string group_id = group.substr(0, 8);
    std::vector<std::string> ids = redis.get_set("group_number:" + group_id);
    nlohmann::json json;
    json["mode"] = j["mode"];
    for (int i = 0; i < ids.size(); i++) {
        if (ids[i].substr(1, -1) == id) {
            if (ids[i].substr(0, 1) == "!" || ids[i].substr(0, 1) == "*") {
                json["y_n"] = "no1";
                return json.dump();
            }

            if (!redis.is_set("group_ban:" + group_id, id)) {
                redis.add_set("group_ban:" + group_id, id);
                json["y_n"] = "yes1";
                return json.dump();
            } else {
                redis.del_set("group_ban:" + group_id, id);
                json["y_n"] = "yes2";
                return json.dump();
            }
            // redis.del_set("group_number:" + group_id, "&" + id);
            // redis.del_set("groupjoin:" + id, group_id);
        }
    }
    json["y_n"] = "no";
    return json.dump();
}
std::string fun_group_will_list(nlohmann::json &j, Redis &redis) {
    nlohmann::json json;
    std::string group_id = j["group_id"];
    std::vector<std::string> ids = redis.get_set("group_will:" + group_id);
    json["mode"] = j["mode"];
    json["ids"] = ids;
    std::vector<std::string> names;
    for (int i = 0; i < ids.size(); i++) {
        //
        names.push_back(redis.get_hash("account:" + ids[i], "name"));
    }
    json["names"] = names;
    return json.dump();
}

std::string fun_group_chuli_will(nlohmann::json &j, Redis &redis) {
    std::string n = j["y_n"];
    std::string id = j["id"];
    std::string group_id = j["group_id"];
    if (n == "no") {
        redis.del_set("group_will:" + group_id, id);
    } else {
        redis.add_set("group_number:" + group_id, "&" + id);
        redis.del_set("group_will:" + group_id, id);
        redis.add_set("groupjoin:" + id, group_id);
    }
    return "";
}
std::string fun_group_his(nlohmann::json &j, Redis &redis) {
    std::string group_id = j["group_id"];
    nlohmann::json jj;
    std::vector<std::string> vec = redis.list_read("group_msg:" + group_id);
    jj["mode"] = GROUP_HIS;
    jj["his"] = vec;
    return jj.dump();
}

bool isJsonString(std::string &str) {
    try {
        nlohmann::json j = nlohmann::json::parse(str);
        return true;
    } catch (nlohmann::json::exception &e) {
        return false;
    }
}

std::string fun_file_send(nlohmann::json &j, Redis &redis, int c_fd) {
    std::string time_str = j["file_time"];
    std::string file_name = j["file_name"];
    std::string will = j["file_will"];
    std::string will_id = j["file_id"];
    std::string path = j["file_path"];
    size_t file_size = j["file_size"];
    std::string id = j["id"];
    //  int fd = j["fd"];
    // 群聊的传输文件
    //
    std::cout << id << "将要向" << will_id << "发送文件" << file_name << std::endl;
    //
    std::filesystem::path dir = "../test/";
    if (!std::filesystem::exists(dir)) {
        try {
            std::filesystem::create_directories(dir);
        } catch (const std::filesystem::filesystem_error &e) {
            std::cerr << "Failed to create directories: " << e.what() << std::endl;
            // 发给客户端
            return "";
        }
    }
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    std::string creatFile = dir.string() + file_name;
    FILE *fp = fopen(creatFile.c_str(), "wb");
    //
    if (fp == NULL) {
        std::cerr << "Failed to open file for writing" << std::endl;
        return "";
    }
    int len;
    char buffer[32768];
    off_t total_received = 0;
    try {
        int flag = fcntl(c_fd, F_GETFL, 0);
        fcntl(c_fd, F_SETFL, flag & ~O_NONBLOCK);
        while (total_received < file_size) {
            len = recv(c_fd, buffer, sizeof(buffer), 0);
            if (len <= 0) {
                if (len < 0) {
                    std::cout << "发送失败" << std::endl;
                    perror("recv");
                }
                fclose(fp);
                return "";
            }
            fwrite(buffer, 1, len, fp);
            total_received += len;
            float progress = static_cast<float>(total_received) / file_size * 100;
            std::cout << progress << "%" << std::endl;
        }
        // fwrite(buffer, 1, len, fp);
        fcntl(c_fd, F_SETFL, flag);
        std::cout << " 接收" << total_received << "字节" << std::endl;
    } catch (...) {
        std::cerr << "An error occurred during file reception" << std::endl;
        fclose(fp);
        return "";
    }
    //// 这里发送成功了
    // 将文件保存到数据库里
    if (will == "g") {
        redis.add_set("group_file:" + will_id.substr(0, 8), file_name);
        // 给群里所有人都通知说可以取
        nlohmann::json json;
        json["mode"] = j["mode"];
        json["will_id"] = will_id;
        json["will"] = will;
        json["file_name"] = file_name;
        json["id"] = id;
        std::vector<std::string> num = redis.get_set("group_number:" + will_id.substr(0, 8));
        for (int i = 0; i < num.size(); i++) {
            if (num[i].substr(1, -1) == id) {
                continue;
            }
            // 如果在线
            std::string fd = redis.get_hash("account:" + num[i].substr(1, -1), "fd");
            if (fd != "-1") {
                send_fd(stoi(fd), json.dump());
            } else {
                // 放在
                nlohmann::json js;
                js["mode"] = "g_file";
                js["id"] = will_id;
                js["name"] = file_name;
                redis.add_set("ac_msg:" + num[i].substr(1, -1), js.dump());
            }
        }

    } else {
        nlohmann::json josn11 = nlohmann::json::parse(will_id);
        std::string id1 = josn11["id"];
        redis.add_set("friend_file:" + id + "_" + id1, file_name);
        redis.add_set("friend_file:" + id1 + "_" + id, file_name);
        nlohmann::json json;
        json["mode"] = j["mode"];
        json["will_id"] = id1;
        json["will"] = will;
        json["file_name"] = file_name;
        json["id"] = id;
        std::string fd = redis.get_hash("account:" + id1, "fd");
        if (fd != "-1") {
            send_fd(stoi(fd), json.dump());
        } else {
            nlohmann::json js;
            js["mode"] = "f_file";
            js["id"] = id1;
            js["name"] = file_name;
            redis.add_set("ac_msg:" + id, js.dump());
        }
    }
    fclose(fp);
    return "";
}

std::string fun_file_receive_g(nlohmann::json &j, Redis &redis) {
    std::string group_id = j["group_id"];
    std::vector<std::string> name = redis.get_set("group_file:" + group_id);
    j["names"] = name;
    return j.dump();
}
std::string fun_file_receive_g1(nlohmann::json &j, Redis &redis) {
    // 这里是发送文件给客户端
    std::string name = j["name"];
    int fd = j["fd"];
    std::string id = j["id"];
    int s_fd = j["s_fd"];
    std::string path = "../test/" + name;
    std::filesystem::path filePath(path);
    // 时间
    // 这里还要查文件还在不再
    if (!std::filesystem::exists(filePath)) {
        std::cout << "文件不存在" << std::endl;
        return "";
    }
    if (!std::filesystem::is_regular_file(filePath)) {
        std::cout << "文件有问题。" << std::endl;
        return "";
    }

    int file_fd = open(path.c_str(), O_RDONLY);
    if (file_fd == -1) {
        std::cout << "文件打不开" << std::endl;
        return "";
    }
    //
    std::time_t now = std::time(nullptr);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
    std::string time_str = ss.str();
    // 然后发个头出去，叫接受文件
    nlohmann::json json1;
    json1["mode"] = FILE_RECEIVE_G1;
    json1["s_fd"] = s_fd;
    json1["id"] = id;
    json1["time"] = time_str;
    json1["file_name"] = name;
    size_t file_size = std::filesystem::file_size(filePath);
    std::string file_name = filePath.filename().string();
    json1["file_size"] = file_size;
    send_fd(fd, json1.dump());
    // 这里发头包
    const size_t BUFFER_SIZE = 32768;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_sent;
    off_t offset = 0;
    size_t total_sent = 0;
    while (total_sent < file_size) {
        ssize_t bytes_to_send = std::min(BUFFER_SIZE, file_size - total_sent);
        bytes_sent = sendfile(fd, file_fd, &offset, bytes_to_send);
        if (bytes_sent <= 0) {
            // std::cerr << "sendfile error" << std::endl;
            // perror("sendfile");
            // break;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 缓冲区已满, 使用非阻塞发送
                bytes_sent = send(fd, buffer, BUFFER_SIZE, MSG_DONTWAIT);
                if (bytes_sent > 0) {
                    total_sent += bytes_sent;
                } else if (bytes_sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    // 缓冲区仍然满, 暂时跳过此次发送
                    continue;
                } else {
                    // 其他错误, 处理异常
                    return "";
                }
            } else {
                // 其他错误, 处理异常
                return "";
            }
        }
        total_sent += bytes_sent;
        float progress = (float)total_sent / file_size * 100;
        std::cout << "Progress: " << progress << "%" << std::endl;
        // std::cout << "发送" << rr << " 字节" << std::endl;
    }
    return "";
}
std::string fun_file_receive_f(nlohmann::json &j, Redis &redis) {
    std::string id = j["id"];
    std::string friend_id = j["friend_id"];
    std::vector<std::string> names = redis.get_set("friend_file:" + id + "_" + friend_id);
    nlohmann::json json;
    json["mode"] = FILE_RECEIVE_F;
    json["names"] = names;
    return json.dump();
}

std::string fun_file_receive(nlohmann::json &j, Redis &redis, int new_fd) {
    std::string name = j["name"];
    std::string path = "../test/" + name;
    //  int new_fd = j["fd"];
    std::ifstream file(path, std::ios::binary);
    //
    std::filesystem::path filePath(path);
    // 时间
    // 这里还要查文件还在不再
    nlohmann::json json;
    json["mode"] = FILE_RECEIVE;
    json["y_n"] = "yes";
    if (!std::filesystem::exists(filePath)) {
        std::cout << "文件不存在" << std::endl;
        json["y_n"] = "no";
        send_fd(new_fd, json.dump());
        std::cout << "发给" << new_fd << ":" << json.dump() << std::endl;
        send_fd(new_fd, json.dump());
        return "";
    }
    if (!std::filesystem::is_regular_file(filePath)) {
        std::cout << "文件有问题。" << std::endl;
        json["y_n"] = "no";
        send_fd(new_fd, json.dump());
        std::cout << "发给" << new_fd << ":" << json.dump() << std::endl;
        send_fd(new_fd, json.dump());
        return "";
    }
    int file_fd = open(path.c_str(), O_RDONLY);
    if (file_fd == -1) {
        std::cout << "文件打不开" << std::endl;
        json["y_n"] = "no";
        send_fd(new_fd, json.dump());
        std::cout << "发给" << new_fd << ":" << json.dump() << std::endl;
        return "";
    }
    size_t file_size = std::filesystem::file_size(filePath);
    json["file_size"] = file_size;
    json["file_name"] = name;
    std::cout << "size:" << file_size << std::endl;
    send_fd(new_fd, json.dump());
    size_t rr = 0;
    int retries = 0;
    std::cout << "发给" << new_fd << ":" << json.dump() << std::endl;

    // if (file.is_open()) {
    //     char buffer[32768];
    //     while () {
    //         if(file.read(buffer, sizeof(buffer))>=)
    //             size_t re = send(new_fd, buffer, file.gcount(), 0); // sizeof(buffer)
    //         if (re == -1) {
    //             //
    //             std::cerr << "Error" << std::endl;
    //             std::cout << "发送" << rr << " 字节" << std::endl;
    //             return "";
    //         }
    //         rr += re;

    //         // std::cout << (double)rr / file_size * 100 << "%" << std::endl;
    //     }
    // std::cout << (double)rr / file_size * 100 << "%" << std::endl;
    //     std::cout << "发送" << rr << " 字节" << std::endl;
    //     send(new_fd, buffer, file.gcount(), 0);
    //     file.close();
    // }
    // else {
    //     std::cerr << "Unable to open file" << std::endl;
    // }
    const size_t BUFFER_SIZE = 32768;
    size_t total_sent = 0;
    size_t bytes_sent = 0;
    off_t offset = 0;
    char buffer[BUFFER_SIZE];
    std::cout << " 开始发送" << std::endl;
    while (total_sent < file_size) {
        ssize_t bytes_to_send = std::min(BUFFER_SIZE, file_size - total_sent);
        bytes_sent = sendfile(new_fd, file_fd, &offset, bytes_to_send);
        if (bytes_sent < 0) {
            std::cerr << "sendfile error" << std::endl;
            perror("sendfile");
            break;
        }
        if (bytes_sent == 0) {
            std::cout << "文件完了" << std::endl;
            break;
        }
        total_sent += bytes_sent;
        float progress = (float)total_sent / file_size * 100;
        std::cout << "Progress: " << progress << "%" << std::endl;
    }
    std::cout << "发送" << total_sent << " 字节" << std::endl;
    file.close();
    return "";
}
