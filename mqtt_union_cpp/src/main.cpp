#include "system_controller.h"
#include <signal.h>
#include <iostream>
volatile sig_atomic_t stop_flag = 0;
SystemController* controller_ptr = nullptr;

void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received." << std::endl;
    stop_flag = 1;
    if (controller_ptr) {
        controller_ptr->stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 종료 대기
    }
}

int main() {
    signal(SIGINT, signalHandler);
    
    try {
        SystemController controller;
        controller_ptr = &controller;  // 시그널 핸들러에서 접근할 수 있도록 포인터 설정
        controller.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}