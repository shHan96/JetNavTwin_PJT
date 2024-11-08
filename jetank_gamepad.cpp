#include <iostream>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include "Jetank/Servos.h"
#include "Jetank/motors.h"

class GamepadController {
private:
    const char* DEVICE_PATH = "/dev/input/js0";
    int joystick_fd;
    Servos servos;
    Motors motors;
    
    // Constants for control
    const double MOVE_START_TOR = 0.15;
    const int SERVO_SPEED = 150;
    const int16_t SERVO_STEP = 5;
    
    // Current positions
    double fb_input = 0;
    double lr_input = 0;
    int16_t servo1_angle = 0;  // 카메라 팬
    int16_t servo2_angle = 0;  // 로봇 팔
    int16_t servo3_angle = 0;  // 3번 서보
    int16_t gripper_angle = 0;
    
    // Movement states
    bool is_servo2_up = false;
    bool is_servo2_down = false;
    bool is_servo3_up = false;    // R1
    bool is_servo3_down = false;  // R2
    bool is_gripper_closing = false;
    bool is_gripper_opening = false;
    
    void initializeServos() {
        servos.setServoAngle(1, 0);  // 카메라 팬
        servos.setServoAngle(2, 0);  // 로봇 팔
        servos.setServoAngle(3, 0);  // 3번 서보
        servos.setServoAngle(4, 0);  // 그리퍼
        servos.setServoAngle(5, 0);
    }

    void processJoystickEvent(const js_event& event) {
        if (event.type & JS_EVENT_BUTTON) {
            switch(event.number) {
                case 6: // L1 - 그리퍼 닫기
                    is_gripper_closing = event.value == 1;
                    is_gripper_opening = false;
                    break;
                case 8: // L2 - 그리퍼 열기
                    is_gripper_opening = event.value == 1;
                    is_gripper_closing = false;
                    break;
                case 4: // Y - 2번 모터 위로
                    if(event.value) {
                        servo2_angle = std::min(80, servo2_angle + SERVO_STEP);
                        servos.setServoAngle(2, servo2_angle);
                    }
                    break;
                case 0: // A - 2번 모터 아래로
                    if(event.value) {
                        servo2_angle = std::max(-80, servo2_angle - SERVO_STEP);
                        servos.setServoAngle(2, servo2_angle);
                    }
                    break;
                case 3: // X - 1번 모터 좌회전
                    if(event.value) {
                        servo1_angle = std::max(-80, servo1_angle - SERVO_STEP);
                        servos.setServoAngle(1, servo1_angle);
                    }
                    break;
                case 1: // B - 1번 모터 우회전
                    if(event.value) {
                        servo1_angle = std::min(80, servo1_angle + SERVO_STEP);
                        servos.setServoAngle(1, servo1_angle);
                    }
                    break;
                case 7: // R1 - 3번 서보 위로
                    if(event.value) {
                        servo3_angle = std::min(80, servo3_angle + SERVO_STEP);
                        servos.setServoAngle(3, servo3_angle);
                    }
                    break;
                case 9: // R2 - 3번 서보 아래로
                    if(event.value) {
                        servo3_angle = std::max(-80, servo3_angle - SERVO_STEP);
                        servos.setServoAngle(3, servo3_angle);
                    }
                    break;
            }
        }
        else if (event.type & JS_EVENT_AXIS) {
            double value = event.value / 32767.0;
            switch(event.number) {
                case 1: // Left stick Y - 전진/후진
                    fb_input = -value;  // 위로 하면 전진, 아래로 하면 후진
                    updateMotors();
                    break;
                case 0: // Left stick X - 좌/우 회전
                    lr_input = -value;  // 왼쪽으로 하면 왼쪽으로, 오른쪽으로 하면 오른쪽으로
                    updateMotors();
                    break;
                case 7: // D-pad Y - 카메라 상하
                    if(std::abs(value) > 0.5) {
                        static int16_t camera_tilt = 0;
                        camera_tilt += value > 0 ? 5 : -5; // 위 누르면 내려가고, 아래 누르면 올라감
                        servos.setServoAngle(5, camera_tilt);
                    }
                    break;
            }
        }
    }

    void updateMotors() {
        if (std::abs(fb_input) < MOVE_START_TOR && std::abs(lr_input) < MOVE_START_TOR) {
            motors.moveStop();
            return;
        }
        
        double left_speed = fb_input - lr_input;   // 방향 수정
        double right_speed = fb_input + lr_input;  // 방향 수정
        
        double max_speed = std::max(std::abs(left_speed), std::abs(right_speed));
        if (max_speed > 1.0) {
            left_speed /= max_speed;
            right_speed /= max_speed;
        }
        
        motors.setSpeed(std::abs(max_speed));
        
        if (left_speed > 0 && right_speed > 0) {
            motors.moveForward();
        } else if (left_speed < 0 && right_speed < 0) {
            motors.moveBackward();
        } else if (left_speed > 0 && right_speed < 0) {
            motors.moveRight();  // 방향 수정
        } else if (left_speed < 0 && right_speed > 0) {
            motors.moveLeft();   // 방향 수정
        }
    }
    
    void updatePositions() {
        // 그리퍼 제어
        if (is_gripper_closing && gripper_angle > -90) {
            gripper_angle -= 2;
            servos.setServoAngle(4, gripper_angle);
        }
        if (is_gripper_opening && gripper_angle < 0) {
            gripper_angle += 2;
            servos.setServoAngle(4, gripper_angle);
        }
    }

public:
    GamepadController() : joystick_fd(-1) {
        joystick_fd = open(DEVICE_PATH, O_RDONLY);
        if (joystick_fd == -1) {
            throw std::runtime_error("Could not open joystick device");
        }
        initializeServos();
    }

    void run() {
        js_event event;
        while (true) {
            if (read(joystick_fd, &event, sizeof(event)) == sizeof(event)) {
                processJoystickEvent(event);
            }
            
            updatePositions();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }

    ~GamepadController() {
        if (joystick_fd != -1) {
            close(joystick_fd);
        }
        motors.moveStop();
    }
};

int main() {
    try {
        GamepadController controller;
        std::cout << "JETANK Gamepad Controller started" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "Left stick: Move robot (Up: forward, Down: backward, Left: turn left, Right: turn right)" << std::endl;
        std::cout << "L1/L2: Close/Open gripper" << std::endl;
        std::cout << "Y: Servo 2 up" << std::endl;
        std::cout << "A: Servo 2 down" << std::endl;
        std::cout << "X: Servo 1 left" << std::endl;
        std::cout << "B: Servo 1 right" << std::endl;
        std::cout << "R1: Servo 3 up" << std::endl;
        std::cout << "R2: Servo 3 down" << std::endl;
        std::cout << "D-pad Up: Camera down" << std::endl;
        std::cout << "D-pad Down: Camera up" << std::endl;
        
        controller.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}