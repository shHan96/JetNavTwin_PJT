#pragma once
#include <torch/script.h>
#include <torch/torch.h>
#include "motors.h"
#include "color_detector.h"
#include "Servos.h"
#include "mqtt_controller.h"
#include <vector>
#include <string>
#include <array>
#include <opencv2/imgcodecs.hpp>
#include "camera_manager.h"
class AutonomousController {
public:
AutonomousController(Motors& motors, MQTTController& mqtt) 
        : robot(motors), mqtt_(mqtt), 
          colorDetector(ColorDetector::TargetColor::YELLOW) {
        
        try {
            // AI 모델 로드
            model = torch::jit::load("best_steering_model_xy_test.pt", torch::kCUDA);
            model.eval();
        }
        catch (const c10::Error& e) {
            throw std::runtime_error("Error loading model: " + std::string(e.what()));
        }

        
    }

    ~AutonomousController() {
        stop();
        if (control_thread.joinable()) {
            control_thread.join();
        }
    }

    void execute() {
        if (control_thread.joinable()) {
            control_thread.join();
        }
        
        running = true;
        paused = false;
        control_thread = std::thread(&AutonomousController::controlLoop, this);
    }

    void stop() {
        running = false;
        paused = false;
        cv.notify_one();
        if (control_thread.joinable()) {
            control_thread.join();
        }
        robot.stop();
    }

    void pause() {
        running = false;
        if (control_thread.joinable()) {
            control_thread.join();
        }
        paused = true;
        robot.stop();
    }

    void resume() {
        if (!running) {
            return;
        }
        if (control_thread.joinable()) {
            control_thread.join();
        }
        running = true;
        paused = false;
        control_thread = std::thread(&AutonomousController::controlLoop, this);
    }

    bool isColorDetected() const { return colorDetected; }
    
    void resetColorDetection() {
        colorDetected = false;
        colorDetector.setTargetColor(ColorDetector::TargetColor::PURPLE);
    }

    void init() {
        colorDetected = false;
        servo.setServoSpeed(5, 100);
        servo.setServoAngle(5, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        servo.setServoSpeed(1, 100);
        servo.setServoAngle(1, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        servo.setServoSpeed(5, 50);
        servo.setServoAngle(5, 60);
    }
    bool isRunning() const { return running; } 
private:
    cv::VideoCapture cap;
    Motors& robot;
    MQTTController& mqtt_;
    Servos servo;
    torch::jit::script::Module model;
    ColorDetector colorDetector;
    std::vector<int> compression_params;
    std::atomic<bool> running{false};
    std::atomic<bool> paused{false};
    std::atomic<bool> colorDetected{false};
    static const char* const base64_chars;
    std::thread control_thread;
    std::mutex mutex;
    std::condition_variable cv;
    std::vector<uchar> compressed_buffer; 
    std::vector<uchar> raw_buffer;  
    std::string encodeToBase64(const std::vector<uchar>& data) {
        std::string encoded;
        encoded.reserve(((data.size() + 2) / 3) * 4);
        
        for (size_t i = 0; i < data.size(); i += 3) {
            uint32_t b = (data[i] & 0xFF) << 16;
            if (i + 1 < data.size()) b |= (data[i + 1] & 0xFF) << 8;
            if (i + 2 < data.size()) b |= (data[i + 2] & 0xFF);

            encoded.push_back(base64_chars[(b >> 18) & 0x3F]);
            encoded.push_back(base64_chars[(b >> 12) & 0x3F]);
            encoded.push_back((i + 1 < data.size()) ? base64_chars[(b >> 6) & 0x3F] : '=');
            encoded.push_back((i + 2 < data.size()) ? base64_chars[b & 0x3F] : '=');
        }
        
        return encoded;
    }
 void controlLoop() {
        cv::Mat frame, frame_resized;
        float angle_last = 0.0;
        const float speed_gain = 0.3;
        const float steering_gain = 0.2;
        const float steering_dgain = 0.0;
        compressed_buffer.reserve(80 * 60 * 3 / 8);
        const auto image_interval = std::chrono::milliseconds(33);
        auto last_image_time = std::chrono::steady_clock::now();
        auto last_sensing_time = std::chrono::steady_clock::now();
        const auto sensing_interval = std::chrono::seconds(1);
        
        robot.stop();
        init();
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        
        while (running && !colorDetected) {
            cv::Mat frame = CameraManager::getInstance().getFrame();
            if (frame.empty()) {
                continue;
            }

            auto current_time = std::chrono::steady_clock::now();

            if (colorDetector.detectColor(frame)) {
                colorDetected = true;
                break;
            }

            // AI 모델 처리 및 제어
            torch::Tensor input = preprocess(frame);
            std::vector<torch::jit::IValue> inputs{input};
            
            at::Tensor output = model.forward(inputs).toTensor();
            float x = output[0][0].item<float>();
            float y = (0.5 - output[0][1].item<float>()) / 2.0;

            // 이미지는 0.1초마다 전송
           
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
            // sensing 데이터는 1초마다 전송
            if (current_time - last_sensing_time >= sensing_interval) {
                mqtt_.publishSensingData(x, y);
                last_sensing_time = current_time;
            }

            float angle = std::atan2(x, y);
            float pid = angle * steering_gain + (angle - angle_last) * steering_dgain;
            angle_last = angle;

            float steering = pid;
            float left_motor = std::max(std::min(speed_gain + steering, 1.0f), 0.0f);
            float right_motor = std::max(std::min(speed_gain - steering, 1.0f), 0.0f);
            
            robot.setSpeed(left_motor, right_motor);
            mqtt_.publishMotorData(left_motor, right_motor);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        robot.stop();
    }
    
    torch::Tensor preprocess(cv::Mat& image) {
    cv::resize(image, image, cv::Size(224, 224));
    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    
    torch::Tensor tensor_image = torch::from_blob(image.data, 
                                                {1, image.rows, image.cols, 3}, 
                                                torch::kByte);
    tensor_image = tensor_image.permute({0, 3, 1, 2});
    tensor_image = tensor_image.to(torch::kFloat32);
    tensor_image = tensor_image.div(255.0);
    
    tensor_image[0][0] = tensor_image[0][0].sub_(0.485).div_(0.229);
    tensor_image[0][1] = tensor_image[0][1].sub_(0.456).div_(0.224);
    tensor_image[0][2] = tensor_image[0][2].sub_(0.406).div_(0.225);
    
    return tensor_image.to(torch::kCUDA);
}
    std::string gstreamer_pipeline(int capture_width = 1280, int capture_height = 720,
                                 int display_width = 224, int display_height = 224,
                                 int framerate = 30, int flip_method = 0) {
        return "nvarguscamerasrc ! "
               "video/x-raw(memory:NVMM), "
               "width=(int)" + std::to_string(capture_width) + ", "
               "height=(int)" + std::to_string(capture_height) + ", "
               "format=(string)NV12, "
               "framerate=(fraction)" + std::to_string(framerate) + "/1 ! "
               "nvvidconv flip-method=" + std::to_string(flip_method) + " ! "
               "video/x-raw, width=(int)" + std::to_string(display_width) + ", "
               "height=(int)" + std::to_string(display_height) + ", "
               "format=(string)BGRx ! "
               "videoconvert ! "
               "video/x-raw, format=(string)BGR ! appsink";
    }
    
};
const char* const AutonomousController::base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";