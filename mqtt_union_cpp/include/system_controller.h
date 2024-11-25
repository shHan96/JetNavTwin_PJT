// system_controller.h
#pragma once
#include "autonomous_controller.h"
#include "game5.h"
#include "mqtt_controller.h"
#include <memory>
#include <atomic>
#include <thread>
#include "camera_manager.h"
#include "battery_monitor.h"
class SystemController {
public:
     SystemController(const std::string& broker_address = "tcp://192.168.164.137:1883") 
        : mqtt_(broker_address),
          autonomousController(motors_, mqtt_) {
        mqtt_.setCommandCallback([this](MQTTController::Command cmd) {
            handleCommand(cmd);
        });
        // 카메라 매니저 초기화
        CameraManager::getInstance().initialize(mqtt_);
        last_image_time = std::chrono::steady_clock::now();
        last_battery_time = std::chrono::steady_clock::now();
    }
        void camera_inter(){
            auto current_time = std::chrono::steady_clock::now();
            if (current_time - last_image_time >= image_interval) {
                // 카메라 매니저가 실행 중인지 확인
                if (CameraManager::getInstance().isRunning()) {
                    if (!CameraManager::getInstance().publishFrame()) {
                        // 발행 실패시 로그
                        mqtt_.publishLog(MQTTController::LogType::ERROR, 
                            "Failed to publish camera frame");
                    }
                }
                last_image_time = current_time;
            }
        }
        void battery_inter(){
            auto current_time = std::chrono::steady_clock::now();
            if (current_time - last_battery_time >= battery_check_interval) {
    try {
        auto status = batteryMonitor.getBatteryStatus();
        
        mqtt_.publishBatteryStatus(status.capacity);
            
            
        
    } catch (const std::exception& e) {
        std::cerr << "Battery monitoring error: " << e.what() << std::endl;
    }
    last_battery_time = current_time;
}}
        
        void run() {
        try {
            mqtt_.start();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            mqtt_.publishLog(MQTTController::LogType::SYSTEM, "System starting in Autonomous Yellow Detection mode");
            
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
                    case SystemState::MQTT_CONTROL:
                        camera_inter();
                        battery_inter();
                        break;
                    default:
                        break;
                }
            }
            
            // 종료 전 정리
            cleanupSystem();
        }
        catch (const std::exception& e) {
            std::string error_msg = "Fatal error in system controller: " + std::string(e.what());
            std::cerr << error_msg << std::endl;
            mqtt_.publishLog(MQTTController::LogType::ERROR, error_msg);
            cleanupSystem();
        }
    }

    void stop() {
        running = false;
    currentState = SystemState::SHUTDOWN;
    
    // 먼저 카메라 매니저 종료
    
    
    // 자율 주행 컨트롤러 종료
    autonomousController.stop();
    // 조이스틱 컨트롤러가 있다면 정리
    if (manualController) {
        isJoystickControl = false;  // 조이스틱 제어 상태 먼저 해제
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        cleanupManualControl();
    }
    CameraManager::getInstance().shutdown();
    // 모터 정지
    motors_.stop();
    
    // MQTT 연결 종료 전 잠시 대기
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    mqtt_.publishLog(MQTTController::LogType::SYSTEM, "System stop called");
    
    // MQTT 종료
    try {
        mqtt_.stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    catch (const std::exception& e) {
        std::cerr << "Error during MQTT shutdown: " << e.what() << std::endl;
    }
    }

    bool isRunning() const { return running; }

private:
    enum class SystemState {
        AUTONOMOUS_YELLOW,
        MANUAL_CONTROL_1,
        AUTONOMOUS_PURPLE,
        MANUAL_CONTROL_2,
        SHUTDOWN,
        MQTT_CONTROL
    };

    SystemState currentState{SystemState::AUTONOMOUS_YELLOW};
    SystemState preState{SystemState::AUTONOMOUS_YELLOW};
    Motors motors_;
    MQTTController mqtt_;
    AutonomousController autonomousController;
    std::unique_ptr<CombinedController> manualController;
    std::atomic<bool> running{true};
    std::atomic<bool> isJoystickControl{true};
    std::chrono::steady_clock::time_point last_image_time;
    BatteryMonitor batteryMonitor;
    const std::chrono::milliseconds image_interval{33};
    std::chrono::steady_clock::time_point last_battery_time;
    const std::chrono::seconds battery_check_interval{1};
    void cleanupSystem() {
        motors_.stop();
        if (manualController) {
            manualController.reset();
        }
        mqtt_.publishLog(MQTTController::LogType::SYSTEM, "System shutdown complete");
        CameraManager::getInstance().shutdown();
        // MQTT 연결 종료를 동기적으로 처리
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            mqtt_.stop();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        catch (const std::exception& e) {
            std::cerr << "Error during MQTT shutdown: " << e.what() << std::endl;
        }
    }


    void handleCommand(MQTTController::Command cmd) {
        // 자율주행 모드일 때의 처리
        if (currentState == SystemState::AUTONOMOUS_YELLOW || 
            currentState == SystemState::AUTONOMOUS_PURPLE) {
            if (cmd == MQTTController::Command::STOP) {
                preState = currentState;
                currentState = SystemState::MQTT_CONTROL;
                autonomousController.pause();
                motors_.stop();
                mqtt_.publishLog(MQTTController::LogType::SYSTEM, "Switched to MQTT control mode");
            }
            else if (cmd == MQTTController::Command::FORCE_QUIT) {
                stop();
            }
            return;
        }

        // 수동 조작 모드에서의 명령 처리
        if (currentState == SystemState::MQTT_CONTROL){
            switch (cmd) {
                case MQTTController::Command::START:
                    currentState = preState;
                    if (currentState == SystemState::AUTONOMOUS_YELLOW || 
                        currentState == SystemState::AUTONOMOUS_PURPLE){
                            mqtt_.publishLog(MQTTController::LogType::SYSTEM, "restart to AUTONOMOUS mode");
                        }else{
                            mqtt_.publishLog(MQTTController::LogType::SYSTEM, "Switched to joystick control mode");
                        }
                    break;
                case MQTTController::Command::GO:
                motors_.setSpeed(0.3, 0.3);
                mqtt_.publishMotorData(0.3, 0.3);
                break;
                
            case MQTTController::Command::HALT:
                motors_.setSpeed(0.0, 0.0);
                mqtt_.publishMotorData(0.0, 0.0);
                break;
                
            case MQTTController::Command::LEFT:
                motors_.setSpeed(-0.3, 0.3);
                mqtt_.publishMotorData(-0.3, 0.3);
                break;
                
            case MQTTController::Command::RIGHT:
                motors_.setSpeed(0.3, -0.3);
                mqtt_.publishMotorData(0.3, -0.3);
                break;
                
            case MQTTController::Command::BACK:
                motors_.setSpeed(-0.3, -0.3);
                mqtt_.publishMotorData(-0.3, -0.3);
                break;
            }
        }

        if (currentState == SystemState::MANUAL_CONTROL_1 || 
            currentState == SystemState::MANUAL_CONTROL_2) {
            switch (cmd) {
                case MQTTController::Command::STOP:
                    preState = currentState;
                    currentState = SystemState::MQTT_CONTROL;
                    isJoystickControl=false;
                    cleanupManualControl();  // 조이스틱 연결 해제
                    motors_.stop();
                    mqtt_.publishLog(MQTTController::LogType::SYSTEM, "Switched to MQTT control mode");
                    break;
                    
                    
                case MQTTController::Command::FORCE_QUIT:
                    stop();
                    break;
                
            }
            return;
        }
    }
    void handleAutonomousMode() {
    try {
        // Method 1: Using std::string
        if(autonomousController.isRunning())
        mqtt_.publishLog(MQTTController::LogType::SYSTEM,
            std::string("Starting autonomous mode: ") + 
            (currentState == SystemState::AUTONOMOUS_YELLOW ? "Yellow" : "Purple") +
            " detection");
        

        
            autonomousController.execute();
        
        while (running && !autonomousController.isColorDetected()&&autonomousController.isRunning()) {
            battery_inter();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        if (running && autonomousController.isColorDetected()) {
            // Fix the similar string concatenation here too
            mqtt_.publishLog(MQTTController::LogType::SYSTEM,
                std::string() + 
                (currentState == SystemState::AUTONOMOUS_YELLOW ? "Yellow" : "Purple") +
                " color detected");
        }
        
        if (running && autonomousController.isColorDetected()) {
            switchState();
        }
    }
    catch (const std::exception& e) {
        std::string error_msg = "Error in autonomous mode: " + std::string(e.what());
        std::cerr << error_msg << std::endl;
        mqtt_.publishLog(MQTTController::LogType::ERROR, error_msg);
        running = false;
    }
}
     void handleManualMode() {
        try {
            initializeManualControl();
            isJoystickControl=true;  // 조이스틱 제어 상태 추적
            
            mqtt_.publishLog(MQTTController::LogType::SYSTEM, 
                           "Entering joystick control mode " + 
                           std::string(currentState == SystemState::MANUAL_CONTROL_1 ? "1" : "2"));
            
            bool modeSwitch = false;
            while (running && !modeSwitch&&isJoystickControl) {
                if (isJoystickControl && manualController) {
                    if (manualController->isStartPressed()) {
                        mqtt_.publishLog(MQTTController::LogType::SYSTEM, "Start button pressed");
                        modeSwitch = true;
                    }
                }
                auto current_time = std::chrono::steady_clock::now();
                if (current_time - last_image_time >= image_interval) {
                    // 카메라 매니저가 실행 중인지 확인
                    if (CameraManager::getInstance().isRunning()) {
                        if (!CameraManager::getInstance().publishFrame()) {
                            // 발행 실패시 로그
                            mqtt_.publishLog(MQTTController::LogType::ERROR, 
                                "Failed to publish camera frame");
                        }
                    }
                    last_image_time = current_time;
                }
                battery_inter();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            cleanupManualControl();
            
            if (modeSwitch) {
                switchState();
            }
        }
        catch (const std::exception& e) {
            std::string error_msg = "Error in manual mode: " + std::string(e.what());
            std::cerr << error_msg << std::endl;
            mqtt_.publishLog(MQTTController::LogType::ERROR, error_msg);
            running = false;
        }
    }

    void switchState() {
        switch (currentState) {
            case SystemState::AUTONOMOUS_YELLOW:
                mqtt_.publishLog(MQTTController::LogType::SYSTEM, "Switching to Manual Control 1");
                currentState = SystemState::MANUAL_CONTROL_1;
                break;
                
            case SystemState::MANUAL_CONTROL_1:
                mqtt_.publishLog(MQTTController::LogType::SYSTEM, "Switching to Autonomous Purple Detection");
                currentState = SystemState::AUTONOMOUS_PURPLE;
                autonomousController.resetColorDetection();
                break;
                
            case SystemState::AUTONOMOUS_PURPLE:
                mqtt_.publishLog(MQTTController::LogType::SYSTEM, "Switching to Manual Control 2");
                currentState = SystemState::MANUAL_CONTROL_2;
                break;
                
            case SystemState::MANUAL_CONTROL_2:
                mqtt_.publishLog(MQTTController::LogType::SYSTEM, "Shutting down system");
                currentState = SystemState::SHUTDOWN;
                break;
                
            default:
                break;
        }
    }

    void initializeManualControl() {
        if (!manualController) {
            try {
                manualController = std::make_unique<CombinedController>(mqtt_);
                mqtt_.publishLog(MQTTController::LogType::SYSTEM, "Manual control initialized");
            }
            catch (const std::exception& e) {
                std::string error_msg = "Failed to initialize manual control: " + std::string(e.what());
                mqtt_.publishLog(MQTTController::LogType::ERROR, error_msg);
                throw;
            }
        }
    }

   void cleanupManualControl() {
        if (manualController) {
            manualController.reset();
            mqtt_.publishLog(MQTTController::LogType::SYSTEM, "Manual control cleaned up");
        }
        
    }
};