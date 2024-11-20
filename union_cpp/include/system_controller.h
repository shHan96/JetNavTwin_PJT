#pragma once
#include "autonomous_controller.h"
#include "game5.h"

class SystemController {
private:
    enum class SystemState {
        AUTONOMOUS_YELLOW,
        MANUAL_CONTROL_1,
        AUTONOMOUS_PURPLE,
        MANUAL_CONTROL_2,
        SHUTDOWN
    };
    SystemState currentState{SystemState::AUTONOMOUS_YELLOW};
    AutonomousController autonomousController;
    std::unique_ptr<CombinedController> manualController;
    std::atomic<bool> running{true};
public:
    void run();
    void handleAutonomousMode();
    void handleManualMode();
    void switchState();
    void initializeManualControl();
    void cleanupManualControl();
    void stop(){
        running=false;
        autonomousController.stop();
    }
};