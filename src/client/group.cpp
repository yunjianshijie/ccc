#include "socket.hpp"

//
void Socket::group_do(std::string id, std::string group_id, std::string group_name, int rank) {
    std::cout << "    与" << group_name << "    群聊界面 " << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "输入 :q 退出群聊" << std::endl;
    // 将状态码转成群聊
    // 写下历史记录
    nlohmann::json js;
    js["mode"] = GROUP_HIS;
    js["group_id"] = group_id;
    this->send_string(js.dump());
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return this->result_ready; });
        this->result_ready = false;
    }
    //
    this->state = GROUP_CHAT;
    this->char_id = group_id;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    while (true) {
        // std::cin >> msg;
        std::string msg;
        // 先清空
        std::getline(std::cin, msg, '\n');
        if (msg.length() == 4095) {
            std::cout << "Input exceeded the maximum length of 4095 characters." << std::endl;
            continue;
        }
        if (msg == "") {
            continue;
        }
        if (msg == ":q") {
            this->state = SUCCESS;
            break;
        }
        nlohmann::json json;
        json["id"] = id;
        json["group_id"] = group_id;
        json["msg"] = msg;
        std::time_t now = std::time(nullptr);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
        std::string time_str = ss.str();
        json["time"] = time_str;
        json["mode"] = GROUP_MESSAGE;
        this->send_string(json.dump());
        std::string iii;
        if (rank == OWNER) {
            iii = "(群主)";
        } else if (rank == MANAGER) {
            iii = "(管理员)";
        } else if (rank == MEMBER) {
            iii = "(群员)";
        }
        std::cout << "[" << time_str << "]" << "" << "你: " << iii;
        std::cout << msg << std::endl;
    }
}
void Socket::group_message(std::string id, std::string group_id, int rank) {
    if (rank == OWNER) {
        this->owner_ma(account.id, group_id);
    } else if (rank == MANAGER) {
        this->ma_ma(account.id, group_id);
    } else {
        chengyuan_ma(account.id, group_id);
    }
}

//

void Socket::owner_ma(std::string id, std::string group_id) {
    while (1) {
        std::cout << "<1> 成员管理" << std::endl;
        std::cout << "<2> 添加/删除管理员" << std::endl;
        std::cout << "<3> 邀请成员" << std::endl;
        std::cout << "<4> 禁言/取消禁言群员" << std::endl;
        std::cout << "<5> 查看群成员列表" << std::endl;
        std::cout << "<6> 退出群聊" << std::endl;
        std::cout << "<7> 查看群聊申请" << std::endl;
        std::cout << "<8> 退出" << std::endl;
        std::string chioce;
        std::cin >> chioce;
        std::string ch;
        if (chioce == "1") {
            // 成员管理
            std::cout << "---------------------------------------" << std::endl;
            std::cout << "输出你想踢出的id：" << std::endl;
            std::cin >> ch;
            nlohmann::json j;
            if (ch == this->account.id) {
                std::cout << "别题自己呀" << std::endl;
                return;
            }
            j["mode"] = GROUP_KICK_O;
            j["id"] = ch;
            j["group"] = group_id;
            this->send_string(j.dump());
        } else if (chioce == "2") {
            set_ma(account.id, group_id);
        } else if (chioce == "3") {
            inv(account.id, group_id);
        } else if (chioce == "4") {
            set_ban_o(account.id, group_id);
        } else if (chioce == "5") {
            chankan_num(account.id, group_id);
        } else if (chioce == "6") {
            un_owner_quik_o(account.id, group_id);
            break;
        } else if (chioce == "8") {
            std::cout << "拜拜 ~" << std::endl;
            break;
        } else if (chioce == "7") {
            apply_GroupList(group_id);
        } else {
            std::cout << "输入错误～" << std::endl;
        }
    }
}

//
void Socket::print_group_manage(std::string message) {
    nlohmann::json json = nlohmann::json::parse(message);
    // 解锁
    int rank = json["rank"];
    // std::cout << "nsjdkfjksdhfjksdhsd" << std::endl;
    {
        std::unique_lock<std::mutex> lock(mtx);
        this->result_ready = true;
        this->buf = std::to_string(rank);
        std::cout << buf << std::endl;
        cv.notify_one();
    }
}

// 群聊群主id !
// 群聊成员id &//可空-> 现在是空的
// 管理员id   *//可空-> 现在是空的
// 申请人员是 %

void Socket::print_group_look(std::string message) {
    nlohmann::json json = nlohmann::json::parse(message);
    std::vector<std::string> num = json["num"];
    std::vector<std::string> names = json["names"];
    for (int i = 0; i < num.size(); i++) {
        if (num[i][0] == '!') {
            std::cout << "(" << i + 1 << ")      " << num[i].substr(1, -1) << "   " << names[i] << "   \b" << "(群主)" << std::endl;
        } else if (num[i][0] == '*') {
            std::cout << "(" << i + 1 << ")      " << num[i].substr(1, -1) << "   " << names[i] << "   \b" << "(管理员)" << std::endl;
        } else if (num[i][0] == '&') {
            std::cout << "(" << i + 1 << ")      " << num[i].substr(1, -1) << "   " << names[i] << "   \b" << "(群员)" << std::endl;
        }
    }
    std::cout << "-----------------------------------" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    {
        std::unique_lock<std::mutex> lock(mtx);
        this->result_ready = true;
        cv.notify_one();
    }
}

void Socket::print_group_message(std::string message) {
    nlohmann::json j = nlohmann::json::parse(message);
    std::string name = j["name"];
    std::string id = j["id"];
    if (id == "-1") {
        std::cout << "你被禁言了，可怜" << std::endl;
        return;
    }
    if (id == "-5") {
        std::cout << "没有这个群了" << std::endl;
        return;
    }
    std::string msg = j["msg"];
    std::string time = j["time"];
    std::string group_id = j["group_id"];
    int rank1 = j["rank"];
    std::string nu;
    if (rank1 == OWNER) {
        nu = "(群主)";
    } else if (rank1 == MANAGER) {
        nu = "(管理员)";
    } else {
        nu = "(群员)";
    }
    //
    if (this->state == GROUP_CHAT && this->char_id == group_id) {
        std::cout << "\033[0m\033[1;33m[" << time << "]" << name << " " << nu;
        std::cout << msg << "\033[0m" << std::endl;
    } else {
        std::cout << "\033[0m\033[1;33m[" << "群聊" << group_id << "有一条消息" << "\033[0m" << std::endl;
    }
}

void Socket::chengyuan_ma(std::string id, std::string group_id) {
    while (1) {
        try {
            std::cout << "<1> 查看群成员" << std::endl;
            std::cout << "<2> 邀请成员" << std::endl;
            std::cout << "<3> 退出群聊" << std::endl;
            std::cout << "<4> 退出" << std::endl;
            std::string in;
            std::cin >> in;
            if (in == "1") {
                chankan_num(account.id, group_id);
            } else if (in == "2") {
                inv(account.id, group_id);
            } else if (in == "3") {
                // 推出群聊
                un_owner_quik(account.id, group_id);
                return;
            } else if (in == "4") {
                break;
            } else {
                std::cout << "输入错误" << std::endl;
            }
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
        }
    }
}

void Socket::inv(std::string id, std::string group_id) {
    std::cout << "---------------------------------------" << std::endl;
    std::cout << "输出你想邀请的id： " << std::endl;
    std::cout << "(只能邀请自己的好友哦～)" << std::endl;
    std::string ch;
    std::cin >> ch;
    nlohmann::json j;
    j["mode"] = GROUP_INVITE;
    j["id"] = ch;
    if (ch == id) {
        std::cout << "别邀请自己呀" << std::endl;
        return;
    }
    j["my_id"] = id;
    j["group"] = group_id;
    this->send_string(j.dump());
}
void Socket::chankan_num(std::string id, std::string group_id) {
    std::cout << "              群成员列表              " << std::endl;
    std::cout << "--------------------------------------" << std::endl;
    nlohmann::json j;
    j["mode"] = GROUP_LOOK_NUM;
    j["group_id"] = group_id;
    this->send_string(j.dump());
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return this->result_ready; });
        this->result_ready = false;
    }
}

void Socket::un_owner_quik(std::string id, std::string group_id) {
    std::cout << "你确定要退出了吗？(y/N)" << std::endl;
    // std::cout << "" << std::endl;
    std::string in;
    std::cin >> in;
    if (in == "y" || in == "Y") {
        //
        //
        nlohmann::json json;
        json["mode"] = GROUP_LEAVE;
        json["id"] = id;
        json["group_id"] = group_id;
        this->send_string(json.dump());
    } else {
        return;
    }
}

void Socket::print_group_kick_o(std::string message) {
    nlohmann::json json = nlohmann::json::parse(message);
    // if (id == "-5") {
    //     std::cout << "没有这个群了" << std::endl;
    //     return;
    // }
    if (json["y_n"] == "yes") {
        std::cout << "删除成功" << std::endl;
    } else {
        std::cout << "群里没有这个人" << std::endl;
    }
}

void Socket::un_owner_quik_o(std::string id, std::string group_id) {
    std::cout << "你确定要退出了吗？(y/N)" << std::endl;
    std::cout << "群主退出解散群聊" << std::endl;
    // std::cout << "" << std::endl;
    std::string in;
    std::cin >> in;
    if (in == "y" || in == "Y") {
        nlohmann::json json;
        json["mode"] = GROUP_LEAVE_O;
        json["id"] = id;
        json["group_id"] = group_id;
        this->send_string(json.dump());
    } else {
        return;
    }
}

void Socket::set_ma(std::string id, std::string group_id) {
    std::cout << "---------------------------------------" << std::endl;
    std::cout << "输出你想设置管理的id： " << std::endl;

    try {
        std::string ch;
        std::cin >> ch;
        if (ch == this->account.id) {
            std::cout << "群主当什么管理" << std::endl;
            return;
        }
        int idd = std::stoi(ch);
        nlohmann::json json;
        json["id"] = ch;
        json["group_id"] = group_id;
        json["mode"] = GROUP_SET_MA;
        this->send_string(json.dump());
    } catch (const std::exception &e) {
        std::cout << "输入数字!🙅" << std::endl;
        // std::cerr << e.what() << '\n';
    }
}

void Socket::print_group_set_ma(std::string message) {
    nlohmann::json json = nlohmann::json::parse(message);
    std::string h = json["y_n"];
    if (h == "no") {
        std::cout << "他不是群成员" << std::endl;
    } else if (h == "yes1") {
        std::cout << "添加成功" << std::endl;
    } else {
        std::cout << "删除成功" << std::endl;
    }
}

void Socket::set_ban_o(std::string id, std::string group_id) {
    std::cout << "---------------------------------------" << std::endl;
    std::cout << "输出你想禁言的id：  \n(如果他以及被禁言那么将解禁)" << std::endl;
    try {
        std::string ch;
        std::cin >> ch;
        if (ch == this->account.id) {
            std::cout << "禁言自己吗？不行哦～" << std::endl;
            return;
        }
        int idd = std::stoi(ch);
        nlohmann::json json;
        json["mode"] = GROUP_SET_BAN_O;
        json["id"] = ch;
        json["group_id"] = group_id;
        this->send_string(json.dump());
    } catch (const std::exception &e) {
        std::cout << "输入数字!🙅" << std::endl;
        // std::cerr << e.what() << '\n';
    }
}

void Socket::print_group_set_ban_o(std::string message) {
    nlohmann::json json = nlohmann::json::parse(message);
    std::string h = json["y_n"];
    if (h == "no") {
        std::cout << "他不是群成员" << std::endl;
    } else if (h == "yse1") {
        std::cout << "禁言成功" << std::endl;
    } else {
        std::cout << "取消禁言成功" << std::endl;
    }
}

void Socket::ma_ma(std::string id, std::string group_id) {
    while (1) {
        std::cout << "<1> 踢出成员" << std::endl;
        std::cout << "<2> 邀请成员" << std::endl;
        std::cout << "<3> 禁言/取消禁言群员" << std::endl;
        std::cout << "<4> 查看群成员" << std::endl;
        std::cout << "<5> 退出群聊" << std::endl;
        std::cout << "<6> 退出" << std::endl;
        std::string chioce;
        std::cin >> chioce;
        std::string ch;
        if (chioce == "1") {
            // 成员管理
            std::cout << "---------------------------------------" << std::endl;
            std::cout << "输出你想踢出的id：" << std::endl;
            std::cin >> ch;
            nlohmann::json j;
            if (ch == this->account.id) {
                std::cout << "别踢自己呀" << std::endl;
                return;
            }
            j["mode"] = GROUP_KICK;
            j["id"] = ch;
            j["group"] = group_id;
            this->send_string(j.dump());
        } else if (chioce == "2") {
            inv(account.id, group_id);
        } else if (chioce == "3") {
            set_ban(account.id, group_id);
        } else if (chioce == "4") {
            chankan_num(account.id, group_id);
        } else if (chioce == "6") {
            std::cout << "拜拜～" << std::endl;
            break;
        } else if (chioce == "5") {
            un_owner_quik(account.id, group_id);
            return;
        } else {
            std::cout << "输入错误～" << std::endl;
        }
    }
}
//
void Socket::set_ban(std::string id, std::string group_id) {
    std::cout << "---------------------------------------" << std::endl;
    std::cout << "输出你想禁言的id：  \n(如果他以及被禁言那么将解禁)" << std::endl;
    try {
        std::string ch;
        std::cin >> ch;
        if (ch == this->account.id) {
            std::cout << "禁言自己吗？不行哦～" << std::endl;
            return;
        }
        int idd = std::stoi(ch);
        nlohmann::json json;
        json["mode"] = GROUP_SET_BAN;
        json["id"] = ch;
        json["group_id"] = group_id;
        this->send_string(json.dump());
    } catch (const std::exception &e) {
        std::cout << "输入数字!🙅" << std::endl;
        // std::cerr << e.what() << '\n';
    }
}
void Socket::print_group_set_ban(std::string message) {
    nlohmann::json json = nlohmann::json::parse(message);
    std::string h = json["y_n"];
    if (h == "no") {
        std::cout << "他不是群成员" << std::endl;
    } else if (h == "no1") {
        std::cout << "权限不够" << std::endl;
    } else if (h == "yse1") {
        std::cout << "禁言成功" << std::endl;
    } else {
        std::cout << "取消禁言成功" << std::endl;
    }
}

void Socket::apply_GroupList(std::string group_id) {
    try {
        nlohmann::json json;
        // 先看申请
        json["mode"] = GROUP_WILL_LIST;
        json["group_id"] = group_id;
        this->send_string(json.dump());
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return this->result_ready; });
            this->result_ready = false;
        }
        // 存在队列里面
        nlohmann::json json2 = nlohmann::json::parse(this->buf);
        std::vector<std::string> ids = json2["ids"];
        std::vector<std::string> names = json2["names"];
        if (ids.size() == 0) {
            std::cout << "没有申请" << std::endl;
            return;
        }
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "    " << "id\t" << "名字\t\t" << std::endl;
        for (int i = 0; i < ids.size(); i++) {
            std::cout << "(" << i + 1 << ")  " << ids[i] << "\t" << names[i] << std::endl;
        }

        std::cout << "输入你想处理的序号" << std::endl;
        std::string ch;
        std::cin >> ch;
        int in = std::stoi(ch);
        if (in >= ids.size() && in <= 0) {
            std::cout << "输入不正确" << std::endl;
            return;
        }
        std::cout << "1.同意2.拒绝" << std::endl;
        std::string ch1;
        std::cin >> ch1;
        nlohmann::json json1;
        json1["mode"] = GROUP_CHULI_WILL;
        json1["id"] = ids[in - 1];
        json1["group_id"] = group_id;
        if (ch1 == "1") {
            json1["y_n"] = "yes";
            this->send_string(json1.dump());
        } else if (ch1 == "2") {
            json1["y_n"] = "no";
            this->send_string(json1.dump());
        } else {
            std::cout << "输入不正确" << std::endl;
            return;
        }
    } catch (const std::exception &e) {
        std::cout << "输出数字" << std::endl;
        std::cerr << e.what() << '\n';
    }
}

void Socket::print_group_will_list(std::string message) {
    nlohmann::json json = nlohmann::json::parse(message);
    {
        std::unique_lock<std::mutex> lock(mtx);
        this->result_ready = true;
        this->buf = message;
        cv.notify_one();
    }
}

void Socket::print_group_his(std::string message) {
    nlohmann::json json = nlohmann::json::parse(message);
    std::vector<std::string> his = json["his"];
    int i = 0;
    for (; i < his.size(); i++) {
        this->print_group_his2(his[i]);
    }
    std::cout << "-----------------以上 " << his.size() << "条历史消息--------------" << std::endl;
    {
        std::unique_lock<std::mutex> lock(mtx);
        this->result_ready = true;
        cv.notify_one();
    }
}
void Socket::print_group_his2(std::string message) {
    nlohmann::json j = nlohmann::json::parse(message);

    // j["id"] = id;
    // json["group_id"] = group_id;
    // json["msg"] = msg;
    // std::cout << message << std::endl;
    std::string msg = j["msg"];
    std::string id = j["id"];
    std::string name = j["name"];
    std::string time = j["time"];
    int rank = j["rank"];
    std::string iii;
    if (rank == OWNER) {
        iii = "(群主)";
    } else if (rank == MANAGER) {
        iii = "(管理员)";
    } else if (rank == MEMBER) {
        iii = "(群员)";
    }
    std::cout << "[" << time << "]" << "" << name << ": " << iii << msg << std::endl;
}