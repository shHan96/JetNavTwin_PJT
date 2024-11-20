#pragma once
#include "Servos.h"
#include "motors.h"
#include <thread>
#include <atomic>
#include <cmath>
#include <linux/joystick.h>
#include <fcntl.h>
#include <unistd.h>

class CombinedController {
private:
    Servos servos;
    Motors motors;
    std::atomic<bool> running{true};
    std::thread control_thread;
    int joystick_fd;

    // Constants
    const double MOVE_START_TOR = 0.15;
    const int SERVO_BASE_SPEED = 100;  // Reduced base speed
    const int SERVO_MAX_SPEED = 300;   // Maximum speed
    const double SERVO_ACCEL = 0.2;    // Acceleration factor
    
    // Button states
    struct {
        std::atomic<bool> y_pressed{false};
        std::atomic<bool> a_pressed{false};
        std::atomic<bool> x_pressed{false};
        std::atomic<bool> b_pressed{false};
        std::atomic<bool> r1_pressed{false};
        std::atomic<bool> r2_pressed{false};
        std::atomic<bool> l1_pressed{false};
        std::atomic<bool> l2_pressed{false};
        std::atomic<bool> dpad_up{false};
        std::atomic<bool> dpad_down{false};
    } button_states;
    std::atomic<bool> start_button_pressed{false};
    // Current state
    struct {
        double fb_input = 0;
        double lr_input = 0;
        int16_t servo1_angle = 0;    // Camera pan
        int16_t servo2_angle = 0;    // Robot arm joint 1
        int16_t servo3_angle = 0;    // Robot arm joint 2
        int16_t servo4_angle = 0;    // Gripper
        int16_t servo5_angle = 0;    // Camera tilt
        
        // Add current speeds for smooth acceleration
        int servo1_speed = 0;
        int servo2_speed = 0;
        int servo3_speed = 0;
        int servo4_speed = 0;
        int servo5_speed = 0;
    } state;

    void controlLoop() {
        while (running) {
            js_event event;
            if (read(joystick_fd, &event, sizeof(event)) == sizeof(event)) {
                processJoystickEvent(event);
            }
            updateServoSpeeds();  // Update speeds continuously
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }

    void updateServoSpeeds() {
        // Update each servo's speed based on button states
        updateServoSpeed(1, button_states.x_pressed, button_states.b_pressed, state.servo1_speed);
        updateServoSpeed(2, button_states.y_pressed, button_states.a_pressed, state.servo2_speed);
        updateServoSpeed(3, button_states.r1_pressed, button_states.r2_pressed, state.servo3_speed);
        updateServoSpeed(4, button_states.l1_pressed, button_states.l2_pressed, state.servo4_speed);
        updateServoSpeed(5, button_states.dpad_up, button_states.dpad_down, state.servo5_speed);
    }

    void updateServoSpeed(int servo_num, bool positive_pressed, bool negative_pressed, int& current_speed) {
        if (positive_pressed || negative_pressed) {
            // Gradually increase speed up to max
            int target_speed = SERVO_MAX_SPEED;
            if (negative_pressed) target_speed = -target_speed;
            
            // Smooth acceleration
            if (std::abs(current_speed) < std::abs(target_speed)) {
                current_speed += (target_speed - current_speed) * SERVO_ACCEL;
            }
        } else {
            // Gradually decrease speed to 0
            current_speed *= (1.0 - SERVO_ACCEL);
            if (std::abs(current_speed) < SERVO_BASE_SPEED * 0.1) {
                current_speed = 0;
            }
        }

        // Update servo speed and angle
        if (current_speed != 0) {
            servos.setServoSpeed(servo_num, std::abs(current_speed));
            int16_t target_angle = (current_speed > 0) ? 80 : -80;
            servos.setServoAngle(servo_num, target_angle);
        } else {
            servos.setServoSpeed(servo_num, 0);
            // Get current position and maintain it
            ServoData data = servos.getServoData(servo_num);
            servos.setServoAngle(servo_num, data.current_angle);
        }
    }

    void processJoystickEvent(const js_event& event) {
        if (event.type & JS_EVENT_BUTTON) {
            handleButton(event.number, event.value);
        }
        else if (event.type & JS_EVENT_AXIS) {
            handleAxis(event.number, event.value / 32767.0);
        }
    }

    void handleButton(uint8_t button, int16_t value) {
        bool is_pressed = value == 1;
        
        switch (button) {
            case 4: // A - Second joint forward
                button_states.a_pressed = is_pressed;
                break;
            case 0: // Y - Second joint backward
                button_states.y_pressed = is_pressed;
                break;
            case 3: // B - Camera pan left
                button_states.b_pressed = is_pressed;
                break;
            case 1: // X - Camera pan right
                button_states.x_pressed = is_pressed;
                break;
            case 7: // R1 - Third joint up
                button_states.r1_pressed = is_pressed;
                break;
            case 9: // R2 - Third joint down
                button_states.r2_pressed = is_pressed;
                break;
            case 6: // L1 - Close gripper
                button_states.l1_pressed = is_pressed;
                break;
            case 8: // L2 - Open gripper
                button_states.l2_pressed = is_pressed;
                break;
            case 11: // start 버튼 테스트
                start_button_pressed = is_pressed;
                break;
        }
    }

    void handleAxis(uint8_t axis, double value) {
        switch (axis) {
        case 1: // Left stick Y - forward/backward
            state.fb_input = -value;  // 이 값을 그대로 사용
            updateMotors();
            break;
            
        case 0: // Left stick X - left/right
            state.lr_input = value;  // 부호 변경 제거
            updateMotors();
            break;
            
        case 7: // D-pad Y - camera tilt
            if (std::abs(value) > 0.5) {
                button_states.dpad_up = value > 0;
                button_states.dpad_down = value < 0;
            } else {
                button_states.dpad_up = false;
                button_states.dpad_down = false;
            }
            break;
    }
    }

    void updateMotors() {
            if (std::abs(state.fb_input) < MOVE_START_TOR && std::abs(state.lr_input) < MOVE_START_TOR) {
        motors.moveStop();
        return;
    }

    // 전진/후진 움직임이 더 큰 경우
    if (std::abs(state.fb_input) > std::abs(state.lr_input)) {
        motors.setSpeed(std::abs(state.fb_input));
        if (state.fb_input > 0) {
            motors.moveForward();
        } else {
            motors.moveBackward();
        }
    }
    // 좌/우 움직임이 더 큰 경우
    else {
        motors.setSpeed(std::abs(state.lr_input));
        if (state.lr_input > 0) {
            motors.moveRight();
        } else {
            motors.moveLeft();
        }
    }
    }

public:
    CombinedController() : servos("/dev/ttyTHS1", 1000000) {
        joystick_fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
        if (joystick_fd == -1) {
            throw std::runtime_error("Could not open joystick");
        }

        // Initialize all servos to zero angle with base speed
        for (int i = 1; i <= 5; i++) {
            servos.setServoAngle(i, 0);
            servos.setServoSpeed(i, SERVO_BASE_SPEED);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Start control thread
        control_thread = std::thread(&CombinedController::controlLoop, this);
    }
    bool isStartPressed() {
        bool current = start_button_pressed;
        if (current) {
            start_button_pressed = false; // 읽은 후 리셋
        }
        return current;
    }
    ~CombinedController() {
        running = false;
        if (control_thread.joinable()) {
            control_thread.join();
        }
        if (joystick_fd != -1) {
            close(joystick_fd);
        }
        motors.moveStop();
    }
};