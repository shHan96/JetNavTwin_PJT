#include "autonomous_controller.h"

AutonomousController::AutonomousController() 
    : colorDetector(ColorDetector::TargetColor::YELLOW) {
    std::string pipeline = gstreamer_pipeline();
    cap.open(pipeline, cv::CAP_GSTREAMER);
    
    if (!cap.isOpened()) {
        throw std::runtime_error("Could not open camera");
    }
    
    try {
        model = torch::jit::load("best_steering_model_xy_test.pt", torch::kCUDA);
        model.eval();
    }
    catch (const c10::Error& e) {
        throw std::runtime_error("Error loading model: " + std::string(e.what()));
    }
}

void AutonomousController::execute() {
    cv::Mat frame;
    float angle_last = 0.0;
    const float speed_gain = 0.3;
    const float steering_gain = 0.2;
    const float steering_dgain = 0.0;
    
    while (running && !colorDetected) {
        cap >> frame;
        if (frame.empty()) {
            continue;
        }

        // Check for target color
        if (colorDetector.detectColor(frame)) {
            colorDetected = true;
            break;
        }

        // Autonomous driving logic
        torch::Tensor input = preprocess(frame);
        std::vector<torch::jit::IValue> inputs{input};
        
        at::Tensor output = model.forward(inputs).toTensor();
        float x = output[0][0].item<float>();
        float y = (0.5 - output[0][1].item<float>()) / 2.0;

        float angle = std::atan2(x, y);
        float pid = angle * steering_gain + (angle - angle_last) * steering_dgain;
        angle_last = angle;

        float steering = pid;
        float left_motor = std::max(std::min(speed_gain + steering, 1.0f), 0.0f);
        float right_motor = std::max(std::min(speed_gain - steering, 1.0f), 0.0f);
        
        robot.setSpeed(left_motor, right_motor);
    }
    
    robot.stop();
}

void AutonomousController::resetColorDetection() {
        // 색상 감지 플래그를 리셋
        colorDetected = false;
        
        // 다음 타겟 색상으로 변경
        colorDetector.setTargetColor(ColorDetector::TargetColor::PURPLE);
        
}
void AutonomousController::init(){
    Servos servo;
    colorDetected = false;
    servo.setServoSpeed(5, 100);
    servo.setServoAngle(5, 0);
    std::chrono::milliseconds(500);
    servo.setServoSpeed(1, 30);
    servo.setServoAngle(1, 0);
    std::chrono::milliseconds(500);
    servo.setServoSpeed(5, 30);
    servo.setServoAngle(5, 60);
}
bool AutonomousController::isColorDetected() const {
        return colorDetected;
}



