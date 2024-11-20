#include "system_controller.h"

void SystemController::handleAutonomousMode() {
    try {
        autonomousController.init();
        autonomousController.execute();
        
        if (autonomousController.isColorDetected()) {
            switchState();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in autonomous mode: " << e.what() << std::endl;
        running = false;
    }
}
void SystemController::initializeManualControl() {
    if (!manualController) {
        try {
            manualController = std::make_unique<CombinedController>();
            std::cout << "Manual control initialized" << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to initialize manual control: " << e.what() << std::endl;
            throw;
        }
    }
}

void SystemController::cleanupManualControl() {
    if (manualController) {
        manualController.reset();
        std::cout << "Manual control cleaned up" << std::endl;
    }
}

void SystemController::handleManualMode() {
    try {
        // 수동 제어 초기화
        initializeManualControl();
        
        std::cout << "Entering manual control mode" << std::endl;
        std::cout << "Press START button to switch mode" << std::endl;
        
        bool modeSwitch = false;
        while (running && !modeSwitch) {
            // Start 버튼 확인
            if (manualController && manualController->isStartPressed()) {
                std::cout << "Start button pressed, switching mode..." << std::endl;
                modeSwitch = true;
            }
            
            // CPU 사용량 감소를 위한 짧은 대기
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        // 현재 수동 제어 정리
        cleanupManualControl();
        
        // 상태 전환
        if (modeSwitch) {
            switchState();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in manual control mode: " << e.what() << std::endl;
        running = false;
    }
}

void SystemController::switchState() {
    switch (currentState) {
        case SystemState::AUTONOMOUS_YELLOW:
            std::cout << "Switching to Manual Control 1" << std::endl;
            currentState = SystemState::MANUAL_CONTROL_1;
            break;
            
        case SystemState::MANUAL_CONTROL_1:
            std::cout << "Switching to Autonomous Purple Detection" << std::endl;
            currentState = SystemState::AUTONOMOUS_PURPLE;
            autonomousController.resetColorDetection();
            break;
            
        case SystemState::AUTONOMOUS_PURPLE:
            std::cout << "Switching to Manual Control 2" << std::endl;
            currentState = SystemState::MANUAL_CONTROL_2;
            break;
            
        case SystemState::MANUAL_CONTROL_2:
            std::cout << "Shutting down system" << std::endl;
            currentState = SystemState::SHUTDOWN;
            break;
            
        default:
            break;
    }
}

void SystemController::run() {
    try {
        std::cout << "System starting in Autonomous Yellow Detection mode" << std::endl;
        
        while (running && currentState != SystemState::SHUTDOWN) {
            switch (currentState) {
                case SystemState::AUTONOMOUS_YELLOW:
                case SystemState::AUTONOMOUS_PURPLE:
                    handleAutonomousMode();
                    break;
                    
                case SystemState::MANUAL_CONTROL_1:
                case SystemState::MANUAL_CONTROL_2:
                    handleManualMode();
                    break;
                    
                default:
                    break;
            }
        }
        
        std::cout << "System shutdown complete" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error in system controller: " << e.what() << std::endl;
    }
}