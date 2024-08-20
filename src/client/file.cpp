#include "socket.hpp"
#include <fstream>
#include <filesystem>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
void Socket::file_do()
{
    while (1)
    {
        std::string path;
        std::cout << "-----------------------------------" << std::endl;
        std::cout << "             <1>文件发送            " << std::endl;
        std::cout << "             <2>文件接收            " << std::endl;
        std::cout << "             <3>文件管理            " << std::endl;
        std::cout << "             <4>退出                " << std::endl;
        std::string ch;
        std::cin >> ch;
        if (ch == "1")
        {
            file_send();
            return;
        }
        else if (ch == "2")
        {
            file_receive();
            return;
        }
        else if (ch == "3")
        {
            return;
        }
        else if (ch == "4")
        {
            return;
        }
        else
        {
            std::cout << "请正确输入!" << std::endl;
        }
    }
}
void Socket::file_receive()
{
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "            <1> 群聊               " << std::endl;
    std::cout << "            <2> 私聊               " << std::endl;
    std::cout << "            <3> 退出               " << std::endl;
    std::string in;
    std::cin >> in;
    if (in == "1")
    {
        file_receive_group();
    }
    else if (in == "2")
    {
        file_receive_friend();
    }
    else
    {
        return;
    }
}
void Socket::file_send()
{
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "            <1> 群聊               " << std::endl;
    std::cout << "            <2> 私聊               " << std::endl;
    std::cout << "            <3> 退出               " << std::endl;
    std::string in11;
    std::cin >> in11;
    if (in11 == "1")
    {
        file_send_group();
    }
    else if (in11 == "2")
    {
        file_send_friend();
    }
    else
    {
        return;
    }
}
void Socket::file_send_group()
{
    nlohmann::json josn;
    josn["mode"] = GROUP_LIST;
    //    josn["id_name"] = group_id_name;
    josn["id"] = this->account.id;
    this->send_string(josn.dump());
    {
        std::unique_lock<std::mutex> lock(this->mtx);
        cv.wait(lock, [this]
                { return result_ready; });
        result_ready = false;
    } // 处理结果
    // int h = 0;
    // std::cout << "this->buf: " << this->buf << std::endl;
    nlohmann::json j = nlohmann::json::parse(this->buf);
    std::vector<std::string> ret = j["group_ids"];
    if (ret.size() == 0)
    {
        std::cout << "你没有群聊，快去加一个吧~" << std::endl;
        return;
    }
    std::cout << "选择你要操作的序号" << std::endl;
    std::string choice;
    std::cin >> choice;
    if (std::stoi(choice) > ret.size())
    {
        std::cout << "输错哩" << std::endl;
        return;
    }
    std::string group = ret[std::stoi(choice) - 1];
    std::string path;
    std::cout << "输入你想传的文件的绝对路径" << std::endl;
    while (1)
    {
        std::getline(std::cin, path, '\n');
        if (path.length() == 4095)
        {
            std::cout << "Input exceeded the maximum length of 4095 characters." << std::endl;
            continue;
        }
        if (path == "")
        {
            continue;
        }
        break;
    }
    std::filesystem::path filePath(path);
    if (!std::filesystem::exists(filePath))
    {
        std::cout << "文件不存在" << std::endl;
        return;
    }
    if (!std::filesystem::is_regular_file(filePath))
    {
        std::cout << "文件有问题" << std::endl;
        return;
    }
    // 开一个线程
    std::cout << "传输....." << std::endl;
    std::thread thread_file([this, path, group]
                            { this->send_now(path, "g", group); });
    thread_file.detach();
}
void Socket::file_send_friend()
{
    nlohmann::json josn;
    josn["mode"] = FRIEND_LIST;
    //    josn["id_name"] = group_id_name;
    josn["id"] = this->account.id;
    this->send_string(josn.dump());
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]
                { return this->result_ready; });
        this->result_ready = false;
    }
    std::vector<std::string> ids = this->message_vec;
    if (ids.size() == 0)
    {
        std::cout << "没有好友" << std::endl;
        return;
    }
    std::cout << "输入你想选择的序号：" << std::endl;
    std::string in;
    std::cin >> in;
    std::string ch_id;
    try
    {
        int ii = std::stoi(in);
        if (ii <= 0 || ii > ids.size())
        {
            throw std::runtime_error("Error");
        }
        ch_id = ids[ii - 1];
    }
    catch (const std::exception &e)
    {
        std::cout << "请正确输入" << std::endl;
        return;
    }
    std::string path;
    std::cout << "输入你想传的文件的绝对路径" << std::endl;
    // 先清空
    while (1)
    {
        std::getline(std::cin, path, '\n');
        if (path.length() == 4095)
        {
            std::cout << "Input exceeded the maximum length of 4095 characters." << std::endl;
            continue;
        }
        if (path == "")
        {
            continue;
        }
        break;
    }
    std::filesystem::path filePath(path);
    if (!std::filesystem::exists(filePath))
    {
        std::cout << "文件不存在" << std::endl;
        return;
    }
    if (!std::filesystem::is_regular_file(filePath))
    {
        std::cout << "文件有问题。" << std::endl;
        return;
    }
    std::cout << "传输....." << std::endl;
    nlohmann::json json = nlohmann::json::parse(ch_id);

    std::thread thread_file([this, path, ch_id]
                            { this->send_now(path, "f", ch_id); });
    thread_file.detach();
    // std::cout << "!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
}

void Socket::send_now(std::string path, std::string will, std::string will_id)
{
    // std::cout << "sndjfsdjhflsd" << std::endl;
    // std::cout << path << std::endl;
    //
    // 后台
    int new_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (new_fd == -1)
    {
        std::cerr << "socket peror" << std::endl;
        return;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->hand); // 大端端口
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.s_addr);
    client_fd = connect(new_fd, (struct sockaddr *)&addr, sizeof(addr));
    // std::cout << "client_fd :" << client_fd << std::endl;
    if (client_fd == -1)
    {
        throw std::runtime_error("Error connecting to server");
    }
    std::filesystem::path filePath(path);
    std::time_t now = std::time(nullptr);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
    std::string time_str = ss.str();
    // 然后发个头出去去接受文件
    size_t file_size = std::filesystem::file_size(filePath);
    std::string file_name = filePath.filename().string();

    nlohmann::json json = {
        {"mode", FILE_SEND},
        {"file_time", time_str},
        {"file_name", file_name},
        {"file_size", file_size},
        {"file_will", will},
        {"file_id", will_id},
        {"file_path", path},
        {"id", account.id},
        {"fd", new_fd}};
    this->send_string1(json.dump(), new_fd);
    // 发送

    std::ifstream file(path, std::ios::binary);
    int flag = fcntl(new_fd, F_GETFL, 0);
    fcntl(new_fd, F_SETFL, flag & ~O_NONBLOCK);
    if (file.is_open())
    {
        char buffer[32768];
        size_t rr;
        size_t ri;
        while (file.read(buffer, sizeof(buffer)))
        {
            ri = send(new_fd, buffer, sizeof(buffer), 0);
            rr += ri;
        }
        ri = send(new_fd, buffer, file.gcount(), 0);
        rr += ri;
        file.close();
        std::cout << "发送" << rr << " 字节" << std::endl;
    }
    else
    {
        std::cerr << "Unable to open file" << std::endl;
    }
    fcntl(new_fd, F_SETFL, flag);
    // close(file_fd);
    close(new_fd);

    std::cout << "发送成功" << std::endl;
    // // 关文件描述符
    // // 发完了再发一个
}

void Socket::print_file_send(std::string message)
{
    nlohmann::json js = nlohmann::json::parse(message);
    std::string will = js["will"];
    if (will == "g")
    {
        std::string group = js["will_id"];
        std::string file_name = js["file_name"];
        std::cout << "  \033[1;34m" << "收到来自群聊" << group.substr(8, -1) << "的一个文件" << file_name << "\033[0m" << std::endl;
    }
    else
    {
        std::string f = js["will_id"];
        std::string file_name = js["file_name"];
        std::cout << "  \033[1;34m" << "收到来自好友" << f << "的一个文件" << file_name << "\033[0m" << std::endl;
    }
}
void Socket::file_receive_group()
{
    nlohmann::json josn;
    josn["mode"] = GROUP_LIST;
    //    josn["id_name"] = group_id_name;
    josn["id"] = this->account.id;
    this->send_string(josn.dump());
    {
        std::unique_lock<std::mutex> lock(this->mtx);
        cv.wait(lock, [this]
                { return result_ready; });
        result_ready = false;
    } // 处理结果
    // int h = 0;
    // std::cout << "this->buf: " << this->buf << std::endl;
    nlohmann::json j = nlohmann::json::parse(this->buf);
    std::vector<std::string> ret = j["group_ids"];
    if (ret.size() == 0)
    {
        std::cout << "你没有群聊，快去加一个吧~" << std::endl;
        return;
    }
    std::cout << "选择你要操作的序号" << std::endl;
    std::string choice;
    std::cin >> choice;
    if (std::stoi(choice) > ret.size())
    {
        std::cout << "输错哩" << std::endl;
        return;
    }
    std::string group = ret[std::stoi(choice) - 1];
    std::string path;
    nlohmann::json json1;
    json1["mode"] = FILE_RECEIVE_G;
    json1["group_id"] = group.substr(0, 8);
    this->send_string(json1.dump());
    {
        std::unique_lock<std::mutex> lock(this->mtx);
        cv.wait(lock, [this]
                { return result_ready; });
        result_ready = false;
    } //
    //
    if (message_vec.size() == 0)
    {
        std::cout << " 这个群没有文件" << std::endl;
        return;
    }
    std::cout << "——————————文件名称——————————" << std::endl;
    for (int i = 0; i < message_vec.size(); i++)
    {
        std::cout << " (" << i + 1 << ")  " << message_vec[i] << std::endl;
    }
    std::cout << "请输入你想要的文件的序号" << std::endl;
    std::string cin1;
    std::cin >> cin1;
    try
    {
        int ii = std::stoi(cin1);
        if (ii <= 0 || ii > message_vec.size())
        {
            throw std::runtime_error("Error");
        }
        // 开始接受文件
        std::string name = message_vec[ii - 1];
        std::thread thread_file([this, name]
                                { this->getget_file(name); });
        thread_file.detach();
    }
    catch (const std::exception &e)
    {
        std::cout << "请正确输入" << std::endl;
    }
}

void Socket::print_file_receive_g(std::string message)
{
    nlohmann::json json = nlohmann::json::parse(message);
    std::vector<std::string> names = json["names"];
    {
        std::unique_lock<std::mutex> lock(mtx);
        this->message_vec = names;
        this->result_ready = true;
        cv.notify_one();
    }
}

void Socket::print_file_receive_g1(std::string message)
{
    // 接受文件
    //
    //
    nlohmann::json json = nlohmann::json::parse(message);
    size_t file_size = json["file_size"];
    std::string file_name = json["file_name"];
    std::filesystem::path dir = "../../tttt/";
    // std::string s_fd = json["s_fd"];
    if (!std::filesystem::exists(dir))
    {
        try
        {
            std::filesystem::create_directories(dir);
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            std::cerr << "Failed to create directories: " << e.what() << std::endl;
            return;
        }
    }
    //
    std::string creatFile = dir.string() + file_name;
    FILE *fp = fopen(creatFile.c_str(), "wb");
    //
    if (fp == NULL)
    {
        std::cerr << "Failed to open file for writing" << std::endl;
        return;
    }
    int len;
    char buffer[32768];
    off_t total_received = 0;
    try
    {
        while (total_received < file_size)
        {
            len = recv(this->server_fd, buffer, sizeof(buffer), 0);
            if (len <= 0)
            {
                if (len < 0)
                {
                    std::cout << "发送失败" << std::endl;
                    perror("recv");
                }
                fclose(fp);
                return;
            }
            fwrite(buffer, 1, len, fp);
            total_received += len;
            float progress = static_cast<float>(total_received) / file_size * 100;
            std::cout << progress << "%" << std::endl;
        }
    }
    catch (...)
    {
        std::cerr << "An error occurred during file reception" << std::endl;
        fclose(fp);
        return;
    }
}

void Socket::file_receive_friend()
{
    nlohmann::json josn;
    josn["mode"] = FRIEND_LIST;
    //    josn["id_name"] = group_id_name;
    josn["id"] = this->account.id;
    this->send_string(josn.dump());
    {
        std::unique_lock<std::mutex> lock(this->mtx);
        cv.wait(lock, [this]
                { return result_ready; });
        result_ready = false;
    } // 处理结果
    //
    std::vector<std::string> list = this->message_vec;

    if (list.size() == 0)
    {
        std::cout << "没有好友" << std::endl;
        return;
    }
    std::cout << "输入你想选择的序号：" << std::endl;
    std::string in;
    std::cin >> in;
    std::string ch_id;
    try
    {
        int ii = std::stoi(in);
        if (ii <= 0 || ii > list.size())
        {
            throw std::runtime_error("Error");
        }

        nlohmann::json json = nlohmann::json::parse(list[ii - 1]);
        ch_id = json["id"];
    }
    catch (const std::exception &e)
    {
        std::cout << "请正确输入" << std::endl;
        return;
    }
    nlohmann::json json1;
    json1["mode"] = FILE_RECEIVE_F;
    json1["friend_id"] = ch_id;
    json1["id"] = this->account.id;
    this->send_string(json1.dump());
    {
        std::unique_lock<std::mutex> lock(this->mtx);
        cv.wait(lock, [this]
                { return result_ready; });
        result_ready = false;
    }
    ///____________________
    if (message_vec.size() == 0)
    {
        std::cout << " 你们间没有文件" << std::endl;
        return;
    }
    std::cout << "——————————文件名称——————————" << std::endl;
    for (int i = 0; i < message_vec.size(); i++)
    {
        std::cout << " (" << i + 1 << ")  " << message_vec[i] << std::endl;
    }
    std::cout << "请输入你想要的文件的序号" << std::endl;
    std::string cin1;
    std::cin >> cin1;
    try
    {
        int ii = std::stoi(cin1);
        if (ii <= 0 || ii > message_vec.size())
        {
            throw std::runtime_error("Error");
        }
        //
        // 开始接受文件,开个线程
        std::string name = message_vec[ii - 1];
        std::thread thread_file([this, name]
                                { this->getget_file(name); });
        thread_file.detach();
    }
    catch (const std::exception &e)
    {
        std::cout << "请正确输入" << std::endl;
    }
}

void Socket::print_file_receive_f(std::string message)
{
    nlohmann::json json = nlohmann::json::parse(message);
    std::vector<std::string> names = json["names"];
    {
        std::unique_lock<std::mutex> lock(mtx);
        this->message_vec = names;
        this->result_ready = true;
        cv.notify_one();
    }
}

void Socket::getget_file(std::string name)
{
    // 建立新连接
    // std::cout << "!!!!!!!!!!!!!!!!!!!!!" << std::endl;

    int new_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (new_fd == -1)
    {
        std::cerr << "socket peror" << std::endl;
        return;
    }
    // std::cout << 1 << std::endl;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->hand); // 大端端口
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.s_addr);
    client_fd = connect(new_fd, (struct sockaddr *)&addr, sizeof(addr));
    // std::cout << "client_fd :" << client_fd << std::endl;
    if (client_fd == -1)
    {
        throw std::runtime_error("Error connecting to server");
    }

    // std::cout << 2 << std::endl;
    //  连上了
    //  nlohmann::json json = nlohmann::json::parse(message);
    nlohmann::json json2;
    // std::string name = json["name"];
    json2["name"] = name;
    json2["fd"] = new_fd;
    json2["mode"] = FILE_RECEIVE;
    // 发送请求
    this->send_string1(json2.dump(), new_fd);
    std::string msg;
    msg = this->receive_message1(new_fd);
    //
    // std::cout << 3 << std::endl;
    nlohmann::json json = nlohmann::json::parse(msg);
    if (json["y_n"] != "yes")
    {
        std::cout << "文件不存在" << std::endl;
        return;
    }
    size_t file_size = json["file_size"];
    std::string file_name = json["file_name"];
    // 创建目录
    std::filesystem::path dir = "../../../tttt/";
    if (!std::filesystem::exists(dir))
    {
        try
        {
            std::filesystem::create_directories(dir);
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            std::cerr << "Failed to create directories: " << e.what() << std::endl;
            return;
        }
    }
    // std::cout << 4 << std::endl;
    std::string creatFile = "../../../tttt/" + file_name;
    // std::cout << 1111 << 5 << std::endl;
    // FILE *fp = fopen(creatFile.c_str(), "wb");
    // //
    int file_fd = open(creatFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd == -1)
    {
        perror("open");
        return;
    }

    // std::cout << 6 << std::endl;
    // if (fp == NULL)
    // {
    //     std::cerr << "Failed to open file for writing" << std::endl;
    //     return;
    // }
    int len;
    off_t total_received = 0;
    try
    {
        std::cout << "size:" << file_size << std::endl;
        std::ofstream file(creatFile, std::ios::binary);
        char buffer[32768];

        ssize_t bytes_read;
        ssize_t rr;
        // std::cout << "收到！" << std::endl;
        int flag = fcntl(new_fd, F_GETFL, 0);
        fcntl(new_fd, F_SETFL, flag | O_NONBLOCK);
        while (total_received < file_size)
        {
            len = recv(new_fd, buffer, sizeof(buffer), 0);
            if (len <= 0)
            {

                if (len < 0)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        continue;
                    }
                    std::cout << "接收失败" << std::endl;
                    perror("recv");
                    return;
                }
                // fclose(fp);
                // return;
            }
            total_received += len;
            // float progress = static_cast<float>(total_received) / file_size * 100;
            // std::cout << progress << "%" << std::endl;
            // std::cout << "len" << len << "read:" << total_received << std::endl;
            ssize_t bytes_written = write(file_fd, buffer, len);
            if (bytes_written == -1)
            {
                perror("write");
                close(file_fd);
                return;
            }
            if (total_received == file_size)
            {
                break;
            }
        }
        // rr += bytes_read;
        //   std::cout << "buf" << buffer << std::endl;
        //  fwrite(buffer, 1, len, fp);

        if (total_received < file_size)
        {
            std::cout << total_received << std::endl;
            std::cout << "接收失败" << std::endl;
            fcntl(new_fd, F_SETFL, flag);
            close(new_fd);
            file.close();
            return;
        }
        fcntl(new_fd, F_SETFL, flag);
        close(new_fd);
        std::cout << "接收" << total_received << " 字节" << std::endl;
        std::cout << "接收成功" << std::endl;
        file.close();
    }
    catch (...)
    {
        std::cerr << "An error occurred during file reception" << std::endl;
        close(file_fd);
        close(new_fd);
        return;
    }
}

// std::string creatFile = dir.string() + file_name;
// FILE *fp = fopen(creatFile.c_str(), "wb");
// //
// if (fp == NULL)
// {
//     std::cerr << "Failed to open file for writing" << std::endl;
//     return "";
// }
// int len;
// char buffer[32768];
// off_t total_received = 0;
// try
// {
//     int flag = fcntl(c_fd, F_GETFL, 0);
//     fcntl(c_fd, F_SETFL, flag & ~O_NONBLOCK);
//     while (total_received < file_size)
//     {
//         len = recv(c_fd, buffer, sizeof(buffer), 0);
//         if (len <= 0)
//         {
//             if (len < 0)
//             {
//                 std::cout << "发送失败" << std::endl;
//                 perror("recv");
//             }
//             fclose(fp);
//             return "";
//         }
//         fwrite(buffer, 1, len, fp);
//         total_received += len;
//         float progress = static_cast<float>(total_received) / file_size * 100;
//         std::cout << progress << "%" << std::endl;
//     }
//     // fwrite(buffer, 1, len, fp);
//     fcntl(c_fd, F_SETFL, flag);
//     std::cout << " 接收" << total_received << "字节" << std::endl;
// }
// catch (...)
// {
//     std::cerr << "An error occurred during file reception" << std::endl;
//     // fcntl(c_fd, F_SETFL, flag);
//     fclose(fp);
//     return "";
//}