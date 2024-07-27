#include <iostream>
#include <nlohmann/json.hpp>
#include "back.hpp"
// #include "../client/json.hpp"
// 在客户端用自己的账户信息，登录后，将账户信息存储在本地
// 服务端将账户信息存储在数据库中
class Account {
    // 公共
public:
    // 账户姓名
    std::string name;
    // 保密问题
    std::string question;
    // 账户的id
    std::string id;

    Account() = default; // 默认
    ~Account() = default;

    Account(std::string name, std::string password, std::string question, std::string answer) :
        name(name), password(password), question(question), answer(answer) {
    }
    Account(std::string name, std::string password, std::string question, std::string answer, std::string id):
        name(name), password(password), question(question), answer(answer), id(id) {
        }
    // 私密
private:
    // 账户的密码
    std::string password;
    // 保密问题的答案
    std::string answer;

public:
    // 一为密码登录，二为问题登录
    bool login(std::string password, int num) {
        if (num == 1) {
            if (this->password == password) {
                return true;
            }
        } else if (num == 2) {
            if (this->answer == password) {
                return true;
            }
        }
        return false;
    }
    // bool login(std::string answer){
    //     if(this->answer == answer){
    //         return true;
    //     }
    //     return false;
    // }
    bool logout() {
        return true;
    }
    // 注册
    std::string get_password(){
        return this->password;
    }
    std::string get_answer() {
        return this->answer;
    }
    bool registerAccount(std::string name, std::string password, std::string question, std::string answer) {
        this->name = name;
        this->password = password;
        this->question = question;
        this->answer = answer;
        return true;
    }
    bool changePassword1(std::string answer) {
        if (answer == this->answer) {
            return true;
        }
        return false;
    }
    bool changePassword2(bool change, std::string password) {
        // 如果答案正确，则可以修改密码
        if (change) {
            this->password = password;
            return true;
        }
        return false;
    }
    bool changeQuestion(std::string question, std::string password) {
        if (this->password == password) {
            this->question = question;
            return true;
        }
        return false;
    }
    bool changeAnswer(std::string answer, std::string password) { //修改答案
        if (this->password == password) {
            this->answer = answer;
            return true;
        }
        return false;
    }



    nlohmann::json toJson(int mode) {
        nlohmann::json j;
        if (mode != -2) {
             j["mode"] = mode; 
        }
        j["name"] = this->name;
        j["password"] = this->password;
        j["question"] = this->question;
        j["answer"] = this->answer;
        return j;
    }

    std::string toJsonString(int  mode) {
        return this->toJson(mode).dump();
    }
    void set_pass(std::string password) {
        this->password = password;
    }
    void set_answer(std::string answer) {
        this->answer = answer;
    }

};

// Account(std::string password) :
//     password(password) {
// }//
