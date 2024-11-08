#include <iostream>
#include <thread>
#include <chrono>
#include "game4.h"

int main() {
    try {
        CombinedController controller;
        std::cout << "JETANK Gamepad Controller started" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "Left stick: Move robot" << std::endl;
        std::cout << "X/B: Camera left/right" << std::endl;
        std::cout << "Y/A: Arm forward/backward" << std::endl;
        std::cout << "R1/R2: Arm up/down" << std::endl;
        std::cout << "L1/L2: Close/Open gripper" << std::endl;
        std::cout << "D-pad: Camera tilt" << std::endl;
        
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}