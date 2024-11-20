#pragma once
#include <torch/script.h>
#include <torch/torch.h>
#include "motors.h"
#include "color_detector.h"
#include "Servos.h"
class AutonomousController {
public:
    AutonomousController();
    void execute();
    bool isColorDetected() const;
    void resetColorDetection();
    void init();
    void stop(){
        running=false;
    }
private:
    cv::VideoCapture cap;
    Motors robot;

    torch::jit::script::Module model;
    ColorDetector colorDetector;
    std::atomic<bool> running{true};
    std::atomic<bool> colorDetected{false};
    
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