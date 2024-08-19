#include "socket.hpp"
#include <future>

void Socket::private_chat(std::string id, std::string friend_id, std::string friend_name, std::string fd)
{ // 页面
    std::cout << "    与" << friend_name << "    私聊界面 " << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "输入 :q 退出私聊" << std::endl;
    // 将状态码转成私聊
    this->state = FRIEND_MESSAGE;
    this->char_id = friend_id;
    // 连接数据库，查看历史消息
    //(直接发消息)
    nlohmann::json json;
    json["send_id"] = id;
    json["id"] = id;
    json["friend_id"] = friend_id;
    json["mode"] = HISTORY_MESSAGE;
    this->send_string(json.dump()); // 发送消息
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]
                { return this->result_ready; });
        this->result_ready = false;
    } // 处理结果
    // std::string history = this->mysql->get_history(id, friend_id);
    if (fd == "-1")
    {
        std::cout << "好友不在线哦～ 为离线对话框" << std::endl;
    }
    // 判断好友在不在线
    // std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    while (true)
    {
        std::string message;
        // std::cin >> msg;
        if (std::getline(std::cin, message, '\n'))
        {
            // std::cout << "message:" << message << ":" << std::endl;
            if (message.length() == 4095)
            {
                std::cout << "Input exceeded the maximum length of 4095 characters." << std::endl;
                continue;
            }
            if (message == ":q")
            {
                break;
            }
            if (message == "")
            {
                continue;
            }
            else
            {
                std::cout << "你 :" << message << std::endl;
                this->send_josn_chat_frined(id, friend_id, message, fd, id); // 发送消息
            }
        }
    }
}

std::string Socket::send_josn_chat_frined(std::string id, std::string friend_id, std::string msg, std::string fd, std::string send_id)
{
    nlohmann::json json;
    json["id"] = id;
    json["friend_id"] = friend_id;
    json["msg"] = msg;
    json["mode"] = FRIEND_MESSAGE;
    json["f_fd"] = fd;
    json["send_id"] = send_id;
    this->send_string(json.dump());
    return json.dump();
}

void Socket::print_friend_message(std::string message)
{
    nlohmann::json json = nlohmann::json::parse(message);
    // 查看用户是否在聊天框里面
    if (this->state == FRIEND_MESSAGE && this->char_id == json["friend_id"])
    {
        // 这里就是在了
        std::string id = json["send_id"];
        std::string msg = json["msg"];
        std::cout << "\033[1;34m" << id << " : " << msg << "\033[0m" << std::endl;
    }
    else
    {
        // 打印一条收到了消息
        std::string id = json["friend_id"];
        std::cout << "\033[1;34m收到一条来自 " << id << " 消息\033[0m" << std::endl;
    }
}

std::string Socket::send_json_friend_shield(std::string friend_id)
{
    nlohmann::json json;
    json["id"] = this->account.id;
    json["friend_id"] = friend_id;
    json["mode"] = FRIEND_SHIELD;
    this->send_string(json.dump());
    return json.dump();
}
std::string Socket::send_json_friend_unshield(std::string friend_id)
{
    nlohmann::json json;
    json["id"] = this->account.id;
    json["friend_id"] = friend_id;
    json["mode"] = FRIEND_UNSHIELD;
    this->send_string(json.dump());
    return json.dump();
}
