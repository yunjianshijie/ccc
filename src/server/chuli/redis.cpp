#include "redis.hpp"

// 连接数据库

Redis::Redis() { //
    try {
        this->rc = redisConnect("127.0.0.1", 6379);
        //  连接本机
        if (this->rc->err) {
            redisFree(this->rc);
            throw std::runtime_error("连接redis服务器失败\n");
        }
        std::cout << "连接redis服务器成功" << std::endl;
        // 连接后将所有用户改成fd -1
        redisReply *reply = (redisReply *)redisCommand(rc, "KEYS account:*");
        std::cout << "所有用户fd置为-1" << std::endl;
        if (reply->type == REDIS_REPLY_ARRAY) {
            for (int i = 0; i < reply->elements; i++) {
                std::cout << reply->element[i]->str << std::endl;
                redisCommand(rc, "HSET %s fd -1", reply->element[i]->str);
            }
        }
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        // if (reply->type != REDIS_REPLY_INTEGER) {
        //     freeReplyObject(reply);
        //     throw std::runtime_error("redisCommand error");
        // }
        freeReplyObject(reply);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
}

std::string Redis::get_new_id() { // 获取新的id
    try {
        std::string id;
        redisReply *reply = (redisReply *)redisCommand(rc, "INCRBY ids 1");
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        if (reply->type != REDIS_REPLY_INTEGER) {
            freeReplyObject(reply);
            throw std::runtime_error("redisCommand error");
        }
        id = std::to_string(reply->integer);
        freeReplyObject(reply);
        return id;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
    return "";
}

std::string Redis::setAccount(Account &account) { // 设置账号
    try {
        std::string new_id = get_new_id(); // 获取新的id
        printf("new_id:%s\n", new_id.c_str());

        std::string key = "account:" + new_id;
        //
        std::string com = "HSET " + key + " name " + account.name + " password " + account.get_password() + " answer " + account.get_answer() + " question " + account.question + " fd  -1 ";
        std::cout << com << std::endl;
        //
        redisReply *reply = (redisReply *)redisCommand(rc, "HSET %s name %s password %s answer %s question %s fd -1", key.c_str(), account.name.c_str(), account.get_password().c_str(), account.get_answer().c_str(), account.question.c_str());
        //
        if (reply == nullptr) {
            throw std::runtime_error("321312redisCommand error");
        }
        if (reply->type == REDIS_REPLY_ERROR) {
            freeReplyObject(reply);
            throw std::runtime_error("12212redisCommand error");
        }
        freeReplyObject(reply);
        return new_id;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return "";
    }
}
std::string Redis::getAccount(std::string id) { // 获on信息

    try {
        std::string key = "account:" + id;
        std::string com = "HGETALL " + key;
        redisReply *reply = (redisReply *)redisCommand(rc, "HGETALL %s", key.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        if (reply->type != REDIS_REPLY_ARRAY) {
            freeReplyObject(reply);
            throw std::runtime_error("redisCommand error");
        }
        //  下层用json解析
        nlohmann::json j;
        for (int i = 0; i < reply->elements; i += 2) {
            j[reply->element[i]->str] = reply->element[i + 1]->str;
        }
        std::cout << j.dump() << std::endl;
        std::string json = j.dump();
        freeReplyObject(reply);
        return json;
        //
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return "";
    }
    return "";
}
std::string Redis::delAccount(std::string &account) { // 删除账号
    return "";
}
std::string Redis::getAccountList() { // 获取账号列表
    return "";
}
std::string Redis::json_setAccount(std::string json) { // json设置账号 ,返回id
    nlohmann::json j = nlohmann::json::parse(json);
    std::string id = get_new_id(); // 获取新的id
    std::string key = "account:" + id;
    std::string value = j.dump();
    std::string name = j["name"];
    std::string password = j["password"];
    std::string answer = j["answer"];
    std::string question = j["question"];
    try {
        redisReply *reply;
        // 用哈希表存储账号信息
        std::string cmd = "HMSET " + key + " name " + name + " password " + password + " answer " + answer + " question " + question;
        // redisReply *reply = (redisReply *)redisCommand(rc, "SET %s %s", key.c_str(), json.c_str());
        reply = (redisReply *)redisCommand(rc, cmd.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        if (reply->type != REDIS_REPLY_STATUS) {
            freeReplyObject(reply);
            throw std::runtime_error("redisCommand error");
        }
        if (strcmp(reply->str, "OK") != 0) {
            freeReplyObject(reply);
            throw std::runtime_error("redisCommand error");
        }
        freeReplyObject(reply);
        return id;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return "";
    }
}
std::string Redis::json_getAccount(std::string id) { // json获取账号信息
    Account account;
    try {
        redisReply *reply;
        std::string cmd = "HGETALL account:" + id;
        reply = (redisReply *)redisCommand(rc, cmd.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        if (reply->type != REDIS_REPLY_ARRAY) {
            throw std::runtime_error("redisCommand error");
        }
        std::string pass;
        std::string answer;
        for (int i = 0; i < reply->elements; i += 2) { // elements 字段表示回复对象中包含的子元素的数量
            if (strcmp(reply->element[i]->str, "name") == 0) {
                account.name = reply->element[i + 1]->str;
            } else if (strcmp(reply->element[i]->str, "password") == 0) {
                pass = reply->element[i + 1]->str;
                account.set_pass(pass);
            } else if (strcmp(reply->element[i]->str, "answer") == 0) {
                answer = reply->element[i + 1]->str;
                account.set_answer(answer);
            } else if (strcmp(reply->element[i]->str, "question") == 0) {
                account.question = reply->element[i + 1]->str;
            }
        }
        freeReplyObject(reply);
        return account.toJsonString(-2);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return "";
    }
}
std::string Redis::json_delAccount(std::string json) { // json删除账号
    try {
        nlohmann::json j = nlohmann::json::parse(json);
        std::string id = j["id"];
        // 获取了 id
        // 不需要再获取数据给客户端了
        // 把id删了 放进 数据库（有序集合）
        redisReply *reply;
        std::string cmd = "DEL account:" + id;
        reply = (redisReply *)redisCommand(rc, cmd.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        if (reply->type != REDIS_REPLY_STATUS) {
            freeReplyObject(reply);
            throw std::runtime_error("redisCommand error");
        }
        if (reply->integer == 0) {
            freeReplyObject(reply);
            return "账号不存在";
        } else {
            freeReplyObject(reply);
            // json_getAccount(id); // 获取账号信息
            return "删除成功";
        }
        // freeReplybject(reply);
        // json_getAccount(id); // 获取账号信息
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }

    return "";
}
// json获取账号列表
std::string Redis::json_getAccountList() {
    return "";
}
// json获取账号在线状态
std::string Redis::josn_state_online(std::string json) {
    try {
        nlohmann::json j = nlohmann::json::parse(json);
        std::string id = j["id"];
        int fd = j["fd"];
        this->set_online(id, std::to_string(fd));
        std::string cmd = "HSET account:" + id + " fd " + std::to_string(fd);
        redisReply *reply;
        reply = (redisReply *)redisCommand(rc, cmd.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        if (reply->type != REDIS_REPLY_STATUS) {
            freeReplyObject(reply);
            throw std::runtime_error("redisCommand error");
        }
        if (strcmp(reply->str, "OK") != 0) {
            freeReplyObject(reply);
            throw std::runtime_error("redisCommand error");
        }
        return "success";
        freeReplyObject(reply);

    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return "";
    }
    return "";
}
std::string Redis::redis_command(std::string cmd) { // redis命令
    try {
        redisReply *reply;
        reply = (redisReply *)redisCommand(rc, cmd.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("1redisCommand error");
        }
        // if (reply->type != REDIS_REPLY_STATUS) {
        //    freeReplyObject(reply);
        //     throw std::runtime_error("2redisCommand error");
        // }
        // if (strcmp(reply->str, "OK") != 0) {
        //     freeReplyObject(reply);
        //     throw std::runtime_error("3redisCommand error");
        // }
        return "success";
        freeReplyObject(reply);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return "";
    }
    return "";
}

std::string Redis::state_offline(std::string id) { // 离线是 将fd置为-1
    try {
        redisReply *reply;
        reply = (redisReply *)redisCommand(rc, "HSET account:%s fd -1", id.c_str());

        if (reply == nullptr) {
            throw std::runtime_error("77redisCommand error");
        }
        // if (reply->type != REDIS_REPLY_STATUS) {
        //     freeReplyObject(reply);
        //     throw std::runtime_error("88redisCommand error");
        // }
        // if (strcmp(reply->str, "OK") != 0) {
        //     freeReplyObject(reply);
        //     throw std::runtime_error("99redisCommand error");
        // }
        freeReplyObject(reply);
        return "";
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return "";
    }
}
std::string Redis::json_getUserInfo(std::string id) {
    try {
        redisReply *reply;
        std::string cmd = "HGETALL account:" + id;

        reply = (redisReply *)redisCommand(rc, cmd.c_str());
        std::string name;
        // std::string password;
        std::string fd;
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        if (reply->type != REDIS_REPLY_ARRAY) {
            freeReplyObject(reply);
            throw std::runtime_error("redisCommand error");
        } else {
            for (int i = 0; i < reply->elements; i += 2) {
                if (strcmp(reply->element[i]->str, "name") == 0) {
                    name = reply->element[i + 1]->str;
                } else if (strcmp(reply->element[i]->str, "fd") == 0) {
                    fd = reply->element[i + 1]->str;
                }
            }
        }
        freeReplyObject(reply);
        nlohmann::json j;
        j["name"] = name;
        if (fd == "") {
            fd = "-2";
        }
        j["id"] = id; // id
        j["fd"] = fd;
        j["mode"] = USER_QUERY;
        return j.dump(); // 返回json字符串
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return "";
    }
}

void Redis::set_online(std::string id, std::string fd) {
    try {
        std::string cmd = "SET fd:" + fd + " " + id;
        std::cout << "cmd :" << cmd << std::endl;
        redisReply *reply = (redisReply *)redisCommand(this->rc, cmd.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("11111redisCommand error");
        }
        if (reply->type != REDIS_REPLY_STATUS) {
            freeReplyObject(reply);
            throw std::runtime_error("222redisCommand error");
        }
        if (strcmp(reply->str, "OK") != 0) {
            freeReplyObject(reply);
            throw std::runtime_error("33333redisCommand error");
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
}

std::string Redis::get_online(std::string fd) {
    try {
        std::string cmd = "GET fd:" + fd;
        redisReply *reply = (redisReply *)redisCommand(this->rc, cmd.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        if (reply->type != REDIS_REPLY_STRING) {
            freeReplyObject(reply);
            throw std::runtime_error("redisCommand error");
        }
        std::string id = reply->str;
        return id;

    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return "";
    }
}

void Redis::set_offline(std::string fd) {
    try {
        std::cout << 1111 << std::endl;
        std::string id = this->get_online(fd);
        if (id == "") {
            return;
        }
        std::cout << "id :" << id << std::endl;

        state_offline(id); // 改变用户信息的状态
        // 删除fd
        std::string cmd = "DEL fd:" + fd;
        std::cout << "cmd :" << cmd << std::endl;
        redisReply *reply = (redisReply *)redisCommand(this->rc, cmd.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("4redisCommand error");
        }
        // if (reply->type != REDIS_REPLY_STATUS) {
        //     freeReplyObject(reply);
        //     throw std::runtime_error("5redisCommand error");
        // }
        // if (strcmp(reply->str, "OK") != 0) {
        //     freeReplyObject(reply);
        //     throw std::runtime_error("6redisCommand error");
        // }
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
}
// 添加好友申请列表
void Redis::apply_friend(std::string id, std::string friend_id, std::string friend_name, std::string msg) {
    try {
        // 用集合来存储好友申请列表
        // 每一共好友一段josn
        nlohmann::json j;
        j["id"] = friend_id;
        j["name"] = friend_name;
        j["msg"] = msg;
        std::string cmd = "SADD apply:" + id + " " + j.dump();
        redisReply *reply = (redisReply *)redisCommand(this->rc, cmd.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        if (reply->type != REDIS_REPLY_STATUS) {
            freeReplyObject(reply);
            throw std::runtime_error("redisCommand error");
        }
        if (strcmp(reply->str, "OK") != 0) {
            freeReplyObject(reply);
            throw std::runtime_error("redisCommand error");
        }
        freeReplyObject(reply);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
}
// 获得好友申请列表
std::string Redis::get_apply_friend(std::string id) {
    try {
        //

        std::string cmd = "SMEMBERS apply:" + id;
        redisReply *reply = (redisReply *)redisCommand(this->rc, cmd.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        if (reply->type != REDIS_REPLY_ARRAY) {
            freeReplyObject(reply);
            throw std::runtime_error("redisCommand error");
        }
        nlohmann::json json_array;
        json_array["mode"] = FRIEND_APPLY_LIST;
        std::vector<std::string> apply_list;

        for (int i = 0; i < reply->elements; i++) {
            // json_array["apply_list"].push_back(reply->element[0]->str);
            // std::cout << reply->element[i]->str << std::endl;
            apply_list.push_back(reply->element[i]->str);
        }
        json_array["apply_list"] = apply_list;
        freeReplyObject(reply);
        std::cout << json_array.dump() << std::endl;
        return json_array.dump();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return "null";
    }
}
// 集合
void Redis::add_friend_list(std::string id, std::string friend_id) {
    try {
        std::string cmd = "SADD friend:" + id + " " + friend_id;
        redisReply *reply = (redisReply *)redisCommand(this->rc, cmd.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        if (reply->type != REDIS_REPLY_STATUS) {
            freeReplyObject(reply);
            throw std::runtime_error("1111redisCommand error");
        }
        if (strcmp(reply->str, "OK") != 0) {
            freeReplyObject(reply);
            throw std::runtime_error("1redisCommand error");
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
}
void Redis::del_apply_friend(std::string id, std::string friend_id) {
    try {
        std::string cmd = " SMEMBERS  apply:" + id;
        redisReply *reply = (redisReply *)redisCommand(this->rc, cmd.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        if (reply->type != REDIS_REPLY_ARRAY) {
            freeReplyObject(reply);
            throw std::runtime_error("redisCommand error");
        }
        if (reply->elements == 0) {
            freeReplyObject(reply);
            return;
        }
        printf("elements:%d\n", reply->elements);
        for (int i = 0; i < reply->elements; i++) {
            nlohmann::json json_array = nlohmann::json::parse(reply->element[i]->str);
            if (json_array["id"] == friend_id) {
                cmd = "SREM apply:" + id + " " + reply->element[i]->str;
                reply = (redisReply *)redisCommand(this->rc, cmd.c_str());
                if (reply == nullptr) {
                    throw std::runtime_error("redisCommand error");
                }
                if (reply->type != REDIS_REPLY_STATUS) {
                    freeReplyObject(reply);
                    throw std::runtime_error("redisCommand error");
                }
            }
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
}

std::string Redis::get_friend_list(std::string id) {
    try {
        std::string cmd = "SMEMBERS friend:" + id;
        redisReply *reply = (redisReply *)redisCommand(this->rc, cmd.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        if (reply->type != REDIS_REPLY_ARRAY) {
            freeReplyObject(reply);
            throw std::runtime_error("redisCommand error");
        }
        std::vector<std::string> friend_list;
        nlohmann::json json_array;
        for (int i = 0; i < reply->elements; i++) {
            std::string friend_id = reply->element[i]->str;
            std::string temp = json_getUserInfo((friend_id));
            friend_list.push_back(temp);
        }
        json_array["friend_list"] = friend_list;
        json_array["mode"] = FRIEND_LIST;
        freeReplyObject(reply);
        return json_array.dump();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
    return "";
}

void Redis::list_write(std::string key, std::string value) {
    try {
        redisReply *reply = (redisReply *)redisCommand(this->rc, "LPUSH %s %s", key.c_str(), value.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        freeReplyObject(reply);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
}
std::vector<std::string> Redis::list_read(std::string key) {
    std::vector<std::string> list;
    try {
        redisReply *reply = (redisReply *)redisCommand(this->rc, "LRANGE %s 0 49 ", key.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        for (int i = reply->elements - 1; i >= 0; i--) {
            list.push_back(reply->element[i]->str);
            // std::cout << reply->element[i]->str << std::endl;
        }
        return list;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
    return list;
}
std::vector<std::string> Redis::key_exist(std::string key) {
    std::vector<std::string> list;
    try {
        redisReply *reply = (redisReply *)redisCommand(this->rc, "KEYS %s", key.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        std::string qian = key;
        for (int i = 0; i < reply->elements; i++) {
            std::string new_key1 = reply->element[i]->str;
            std::cout << new_key1 << std::endl;
            std::string new_key = new_key1.substr(qian.length() - 1); // key.substr(.length());
            std::cout << new_key << std::endl;
            list.push_back(new_key);
        }
        return list;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
    return list;
}
std::string Redis::list_del(std::string key) {
    try {
        redisReply *reply = (redisReply *)redisCommand(this->rc, "DEL %s", key.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        std::string value = reply->str;
        freeReplyObject(reply);
        return value;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
    return "";
}

void Redis::add_set(std::string key, std::string value) {
    try {
        redisReply *reply = (redisReply *)redisCommand(this->rc, "SADD %s %s", key.c_str(), value.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        freeReplyObject(reply);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
}
void Redis::del_set(std::string key, std::string value) {
    try {
        redisReply *reply = (redisReply *)redisCommand(this->rc, "SREM %s %s", key.c_str(), value.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        freeReplyObject(reply);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
}
bool Redis::is_set(std::string key, std::string value) {
    try {
        redisReply *reply = (redisReply *)redisCommand(this->rc, "SISMEMBER %s %s", key.c_str(), value.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        if (reply->integer == 1) {
            return true;
        }
        freeReplyObject(reply);
        return false;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return false;
    }
    return false;
}
std::string Redis::get_group_id() // 获取群id
{
    // 8位数，2个4位
    try {
        std::string id;
        while (1) {
            srand((unsigned)time(nullptr));
            for (int i = 0; i < 2; i++) {
                id += std::to_string(rand() % 9000 + 1000); // 生成4位随机数
            }
            std::string key = "group_ids";
            if (this->is_set(key, id)) {
                continue;
            } else {
                break;
            }
        }
        return id;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return "";
    }
}

void Redis::set_hash(std::string key, std::string field, std::string value) {
    try {
        redisReply *reply = (redisReply *)redisCommand(this->rc, "HSET %s %s %s", key.c_str(), field.c_str(), value.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        freeReplyObject(reply);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
}
std::vector<std::string> Redis::get_set(std::string key) {
    // 获取set中的所有元素
    //
    std::vector<std::string> res;
    try {
        redisReply *reply = (redisReply *)redisCommand(this->rc, "SMEMBERS %s", key.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }

        for (int i = 0; i < reply->elements; i++) {
            res.push_back(reply->element[i]->str);
        }
        freeReplyObject(reply);
        return res;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
    return res;
}

std::string Redis::get_hash(std::string key, std::string field) {
    try {
        redisReply *reply = (redisReply *)redisCommand(this->rc, "HGET %s %s", key.c_str(), field.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        std::string res = reply->str;
        freeReplyObject(reply);
        return res;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
    return "";
}
std::vector<std::string> Redis::find_key(std::string key) {
    std::vector<std::string> res;
    try {
        redisReply *reply = (redisReply *)redisCommand(this->rc, "KEYS %s*", key.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }

        for (int i = 0; i < reply->elements; i++) {
            std::cout << reply->element[i]->str << std::endl;
            res.push_back(reply->element[i]->str);
        }
        freeReplyObject(reply);
        return res;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        printf("error");
        return res;
    }
}
bool Redis::key_bool(std::string key) {
    try {
        redisReply *reply = (redisReply *)redisCommand(this->rc, "EXISTS", key.c_str());
        if (reply == nullptr) {
            throw std::runtime_error("redisCommand error");
        }
        if (reply->integer == 1) {
            return true;
        } else {
            return false;
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return false;
    }
}