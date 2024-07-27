//  #include "ui/ui.hpp"
#include "socket.hpp"
#include <cstring>
#include <string>
#include <csignal>
#include <future>
#include <termios.h>
#include <unistd.h>
#include <fstream>
//

int main(int argc, char **argv) {
    // 信号处理
    // std::signal(SIGINT, sig_handler);
    // std::signal(SIGTERM, sig_handler);
    // 创建套接字，并检验传输
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_cc[VEOF] = _POSIX_VDISABLE;
    tcsetattr(STDOUT_FILENO, TCSANOW, &term);
    // EOF
    // signal(SIGINT, SIG_IGN);  // c
    signal(SIGTSTP, SIG_IGN); // z
    signal(SIGQUIT, SIG_IGN); //
    int hand = HAND;
    //char *argv1 = "0.0.0.0";
    if (argc == 2) {
        std::string top = argv[1];
        hand = std::stoi(top);
    }
    if (argc == 3) {
        std::string top = argv[1];
        hand = std::stoi(top);
        //argv1 = argv[2];
    }
    Socket client(hand, argv[2]); // 创建套接字并连接"0.0.0.0"
    client.socket_do();         // 进行传输
    return 0;
    //
}
