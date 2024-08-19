#include "socket.hpp"

Socket::Socket(int handle, char ip[30]) {
    try {
        this->hand = handle;
        this->ip = ip;
        mode = SUCCESS;
        this->server_fd = socket(AF_INET, SOCK_STREAM, 0); // 这里其实 是server_fd
        //   自己的fd
        if (server_fd < 0) {
            throw std::runtime_error("Error creating socket");
        }
        addr.sin_family = AF_INET;
        addr.sin_port = htons(handle); // 大端端口
        inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);
        client_fd = connect(server_fd, (struct sockaddr *)&addr, sizeof(addr));
        std::cout << "client_fd :" << client_fd << std::endl;
        if (client_fd == -1) {
            throw std::runtime_error("Error connecting to server");
        }
        // client_fd = dup(STDIN_FILENO);
        // std::cout << "client_fd :" << client_fd << std::endl;
        begin();
        int number = 0;
        std::cout << "检测通信正常..." << std::endl;
        nlohmann::json j;
        j["mode"] = 0;
        j["c_fd"] = this->client_fd;
        std::string s = j.dump();

        ssize_t lee = send_meg(server_fd, s);
        if (lee == -1) {
            std::cerr << "Failed to send data" << std::endl;
            printf("发送失败");
            throw std::runtime_error("Error sending data");
        } else if (lee < s.size()) {
            printf("发送少字");
            throw std::runtime_error("Error sending data");
        }
        std::cout << "发送正常..." << std::endl;
        std::string buf;                   // 接收数据
        int len = recvMsg(server_fd, buf); // 接收数据
        std::cout << "接收到的数据为：" << buf << std::endl;
        nlohmann::json j1;
        j1 = nlohmann::json::parse(buf);
        if (j1["mode"] == 0) {
            this->client_fd = j1["c_fd"];
        }
        if (len == -1) {
            printf("接受失败");
            throw std::runtime_error("Error receiving data");
        }
        this->buf = buf;
        // main_t();
        this->state = SUCCESS;
        int s_mode = SUCCESS;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        exit(1);
    }
}

void Socket::socket_do() {
    // std::string c_get;
    // c_get = this->receive_string();
    // std::cout << "Received: " << c_get << std::endl;
    begin();
    Account account; // 本机账号信息
    try {
        do {
            std::string choice = register_ui();
            if (choice == "1") { // 登录
                Account account1;
                login_ui(account1);
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //  清理缓存区
                this->send_json_usr_login(account1);                                // 发送                                  // 发送json
                account1.id = "-1";
                account1 = this->receive_json_usr_login(); // 接收json
                                                           //  std::cout << "account1.id =" << account1.id << std::endl;
                if (account1.id == "-2") {
                    std::cout << "没有此账号" << std::endl;
                    continue;
                } else if (account1.id == "-1") {
                    std::cout << "密码错误" << std::endl;
                    continue;
                } else if (account1.id == "-3") {
                    std::cout << "账号已登录" << std::endl;
                } else {
                    // std::cout << "登录成功" << std::endl;
                    this->account = account1;
                    this->send_josn_login_success();
                    user_run();
                    //      std::cout << "dfsfsf" << std::endl;
                    continue;
                }
                // 给服务器发送登录请求
                //  从redis里面找到账号信息
                //
            } else if (choice == "2") {                                             // 注册
                Account account1;                                                   //
                register_ui1(account1);                                             //
                this->send_json_usr_register(account1);                             // 发送json
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 清理缓存区
                // 要接收服务器返回的id
                std::string id = this->receive_json_usr_id();
                if (!(id.empty() || id == "error")) { // 如果id不为空
                    remember_id(id);                  // 记住id
                } else {
                    std::cout << "注册失败" << std::endl;
                    continue;
                }
                std::cout << "注册成功" << std::endl;
            } else if (choice == "3") {
                exit(0);
            } else if (choice == "4") {
                // 修改密码
                //
                if (change_pass()) {
                    std::cout << "修改成功，快去试试吧~~~~";
                }
            } else {
                std::cout << "输入错误，请重新输入" << std::endl;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
        } while (true);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
}

bool Socket::remember_id(std::string id) { // 是否记住id了
    std::string yes = "";
    do {
        std::cout << "你的id是:" << id << std::endl;
        std::cout << "是否记住id(y/n):" << std::endl;
        std::cin >> yes;
    } while (yes != "y");
    return true;
}

std::string Socket::receive_json_usr_id() { // 接收 注册json数据
    try {
        std::string buf = this->receive_string();
        // std::cout << "接收字符 :" << buf << std::endl;
        this->buf = buf;
        if (buf != "") {
            nlohmann::json j = nlohmann::json::parse(buf);
            if (j["mode"] == REGISTER) {
                std::string id = j["id"];
                return id;
            }
        }
        return buf; // 接受回来的数据
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return "error";
    }
}

bool Socket::send_string(std::string chuan) {
    try {
        // std::cout << chuan << std::endl;
        int len = send_meg(server_fd, chuan);
        if (len == -1) {
            throw std::runtime_error("Error sending data");
        } else {
            if (len < chuan.size()) {
                throw std::runtime_error("Error sending data");
                // 传大东西这里用多次传那种吧
            }
        }

    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return false;
    }
    return true;
}

bool Socket::send_string1(std::string chuan, int fd) {
    try {
        // std::cout << chuan << std::endl;
        int len = send_meg(fd, chuan);
        if (len == -1) {
            throw std::runtime_error("Error sending data");
        } else {
            if (len < chuan.size()) {
                throw std::runtime_error("Error sending data");
                // 传大东西这里用多次传那种吧
            }
        }

    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return false;
    }
    return true;
}
std::string Socket::receive_string() { // 接受数据

    try {
        // char buf[1024];
        // memset(buf, 0, sizeof(buf));
        // int len = recv(server_fd, buf, sizeof(buf), 0);
        std::string buf;
        int len = recvMsg(server_fd, buf);
        if (len == -1) {
            throw std::runtime_error("Error receiving data");
        }
        // std::cout << "接受正常..." << std::endl;
        // std::cout << "接受字符 ：" << buf << std::endl;
        std::string mm = buf;
        this->buf = mm;
        if (mm == "error") {
            return "exit"; // 退出
        }
        return mm; // 接受回来的数据
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return "exit";
    }
    return "";
}

bool Socket::send_json_usr_register(Account &account) {
    try {
        std::string json_str;
        nlohmann::json j;
        j["mode"] = REGISTER; // 登录模式 注册模式
        j["name"] = account.name;
        j["password"] = account.get_password();
        j["question"] = account.question;
        j["answer"] = account.get_answer();
        //
        json_str = j.dump();
        return send_string(json_str); // 发送
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return false;
    }
}

Account Socket::receive_json_usr_login() { // 接受 登录 数据 （是否成功登录）
    // 发josn 数据 要有 name password question answer
    Account account1;
    try {
        std::string str = this->receive_string(); //
                                                  //  std::cout << "receive_json_usr_login()接收字符 :" << str << std::endl;
        if (str == "exit") {
            account1.id = "-1";
            return account1;
        }
        nlohmann::json j = nlohmann::json::parse(str);
        if (j["mode"] == LOGIN) {
            account1.name = j["name"];
            account1.set_pass(j["password"]);
            account1.question = j["question"];
            account1.set_answer(j["answer"]);
            account1.id = j["id"];
            return account1;
        } else {
            throw std::runtime_error("Error receiving data");
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        account1.id = "-1";
        return account1;
    }
}

std::string Socket::send_json_usr_login(Account &account) {
    try {
        std::string json_str;
        nlohmann::json j;
        j["mode"] = LOGIN; // 登录模式
        j["id"] = account.id;
        j["password"] = account.get_password();
        // 直接发了
        json_str = j.dump();
        send_string(json_str); // 发送
        return j.dump();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return "exit";
    }
}

bool Socket::error_string(std::string str) // 错误信息
{
    if (str == "error") {
        return true;
    }
    return false;
}
bool Socket::change_pass() // 修改密码
{
    try {
        nlohmann::json json;
        json["mode"] = CHANGE_PASS;
        std::cout << "------------------------------------------" << std::endl;
        std::cout << "请输入你的账号id : " << std::endl;
        std::string id;
        std::cin >> id;
        account.id = id;
        nlohmann::json json1;
        json["id"] = id;
        json1["id"] = id;
        json1["mode"] = CHANGE_PASS1;
        this->send_string(json.dump());
        std::string i = this->receive_string();
        nlohmann::json j = nlohmann::json::parse(i);
        std::cout << "------------------------------------------" << std::endl;
        std::cout << "你的保密问题： " << std::endl;
        std::string qus = j["question"];
        account.question = qus;
        std::cout << qus << std::endl;
        std::cout << "请输入你的答案: " << std::endl;
        std::string answer;
        std::cin >> answer;
        if (j["answer"] != answer) {
            std::cout << "答案错误" << std::endl;
            return false;
        }
        std::cout << "答案正确" << std::endl;
        std::cout << "------------------------------------------" << std::endl;
        std::cout << "请输入你的新密码: " << std::endl;
        std::string pass;
        std::cin >> pass;
        json1["password"] = pass;
        std::cout << "------------------------------------------" << std::endl;
        this->send_string(json1.dump());
        return true;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return false;
    }
}

//
//

//
//
std::string Socket::send_josn_login_success() { // std::string id, int fd
    // LOGIN_SUCCESS
    int fd = this->client_fd;
    std::string id = this->account.id;
    nlohmann::json json;
    json["mode"] = LOGIN_SUCCESS;
    json["id"] = id;
    json["fd"] = fd;
    send_string(json.dump());
    return json.dump(); // 成功
    // return json.dump();
}

std::string Socket::send_json_usr_exit() {
    try {
        nlohmann::json j;
        j["mode"] = EXIT;           // 登录模式
        j["id"] = this->account.id; // account.id;
        send_string(j.dump());      // 发送
                                    // printf("dsfssf\n");
        return j.dump();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
    return "";
}
