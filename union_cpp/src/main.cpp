#include "system_controller.h"
#include <signal.h>

volatile sig_atomic_t stop_flag = 0;
SystemController controller;
void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received." << std::endl;
    stop_flag = 1;
    controller.stop();
    exit(1);
}

int main() {
    signal(SIGINT, signalHandler);
    
    try {
        
        controller.run();
        
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}