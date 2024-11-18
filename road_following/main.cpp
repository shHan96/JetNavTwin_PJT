#include <opencv2/opencv.hpp>
#include <torch/script.h>
#include <torch/torch.h>
#include <iostream>
#include <signal.h>
#include "motors.h"

// 전역 변수로 종료 플래그 추가
volatile sig_atomic_t stop_flag = 0;

class RoadFollower {
private:
    cv::VideoCapture cap;
    Motors robot;
    torch::jit::script::Module model;
    float speed_gain = 0.3;
    float steering_gain = 0.2;
    float steering_dgain = 0.0;
    float steering_bias = 0.0;
    float angle_last = 0.0;
    
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

    torch::Tensor preprocess(cv::Mat& image) {
        cv::resize(image, image, cv::Size(224, 224));
        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        
        torch::Tensor tensor_image = torch::from_blob(image.data, {1, image.rows, image.cols, 3}, torch::kByte);
        tensor_image = tensor_image.permute({0, 3, 1, 2});
        tensor_image = tensor_image.to(torch::kFloat32);
        tensor_image = tensor_image.div(255.0);
        
        tensor_image[0][0] = tensor_image[0][0].sub_(0.485).div_(0.229);
        tensor_image[0][1] = tensor_image[0][1].sub_(0.456).div_(0.224);
        tensor_image[0][2] = tensor_image[0][2].sub_(0.406).div_(0.225);
        
        return tensor_image.to(torch::kCUDA);
    }

public:
    // 안전한 종료를 위한 cleanup 메서드 추가
    void cleanup() {
        std::cout << "\nCleaning up resources..." << std::endl;
        robot.stop();
        cap.release();
        cv::destroyAllWindows();
    }

    RoadFollower() {
        std::string pipeline = gstreamer_pipeline();
        cap.open(pipeline, cv::CAP_GSTREAMER);
        
        if (!cap.isOpened()) {
            std::cerr << "Error: Could not open camera" << std::endl;
            exit(1);
        }
        std::cout << "Camera initialized successfully" << std::endl;
        
        try {
            std::cout << "Loading model..." << std::endl;
            model = torch::jit::load("best_steering_model_xy_test.pt", torch::kCUDA);
            model.eval();
            std::cout << "Model loaded successfully" << std::endl;
        }
        catch (const c10::Error& e) {
            std::cerr << "Error loading model: " << e.what() << std::endl;
            exit(1);
        }
    }

    void execute() {
        cv::Mat frame;
        float angle = 0.0;
        
        while (!stop_flag) {  // stop_flag 체크
            cap >> frame;
            if (frame.empty()) {
                std::cerr << "Error: Could not read frame" << std::endl;
                break;
            }

            torch::Tensor input = preprocess(frame);
            std::vector<torch::jit::IValue> inputs;
            inputs.push_back(input);
            
            at::Tensor output = model.forward(inputs).toTensor();
            float x = output[0][0].item<float>();
            float y = (0.5 - output[0][1].item<float>()) / 2.0;

            angle = std::atan2(x, y);
            float pid = angle * steering_gain + (angle - angle_last) * steering_dgain;
            angle_last = angle;

            float steering = pid + steering_bias;
            
            float left_motor = std::max(std::min(speed_gain + steering, 1.0f), 0.0f);
            float right_motor = std::max(std::min(speed_gain - steering, 1.0f), 0.0f);
            
            robot.setSpeed(left_motor, right_motor);

            std::cout << "\rX: " << x << " Y: " << y << " Angle: " << angle 
                      << " Steering: " << steering << std::flush;

            char key = cv::waitKey(1);
            if (key == 'q') break;
        }
        
        // 루프 종료 후 정리
        cleanup();
    }

    ~RoadFollower() {
        cleanup();
    }
};

// SIGINT 핸들러 함수
void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received." << std::endl;
    stop_flag = 1;  // 안전한 종료를 위해 플래그 설정
}

int main() {
    std::cout << "Road Following Demo" << std::endl;
    
    // SIGINT 핸들러 등록
    signal(SIGINT, signalHandler);
    
    RoadFollower follower;
    follower.execute();
    
    return 0;
}