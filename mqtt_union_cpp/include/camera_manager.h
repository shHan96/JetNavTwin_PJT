// camera_manager.h
#pragma once
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <memory>
#include <mutex>
#include "mqtt_controller.h"

class CameraManager {
public:
    static CameraManager& getInstance() {
        static CameraManager instance;
        return instance;
    }

    void initialize(MQTTController& mqtt) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) {
            mqtt_ = &mqtt;
            std::string pipeline = gstreamer_pipeline(1280, 720, 320, 240, 60);
            if (!cap_.open(pipeline, cv::CAP_GSTREAMER)) {
                throw std::runtime_error("Could not open camera");
            }
            
            compression_params_.push_back(cv::IMWRITE_JPEG_QUALITY);
            compression_params_.push_back(50);
            compression_params_.push_back(cv::IMWRITE_JPEG_OPTIMIZE);
            compression_params_.push_back(1);
            
            initialized_ = true;
            running_ =true;
        }
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
        if (initialized_ && cap_.isOpened()) {
            cap_.release();
            initialized_ = false;
            if (mqtt_) {
                mqtt_->publishLog(MQTTController::LogType::SYSTEM, "Camera system shutdown completed");
            }
        }
    }

    cv::Mat getFrame() {
        std::lock_guard<std::mutex> lock(mutex_);
        cv::Mat frame;
        if (initialized_ && cap_.isOpened()) {
            cap_.read(frame);
        }
        return frame;
    }

    bool publishFrame() {
        if (!running_) return false;  // 종료 신호 확인

        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_ || !mqtt_) return false;

        try {
            cv::Mat frame, frame_resized;
            
            if (cap_.read(frame)) {
                cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
                cv::resize(frame, frame_resized, cv::Size(240, 240));
                compressed_buffer_.clear();
                cv::imencode(".jpg", frame_resized, compressed_buffer_, compression_params_);
                std::string base64_data = encodeToBase64(compressed_buffer_);
                mqtt_->publishImageData(base64_data);
                return true;
            }
        } catch (const std::exception& e) {
            if (mqtt_) {
                mqtt_->publishLog(MQTTController::LogType::ERROR, 
                    "Error in camera frame capture: " + std::string(e.what()));
            }
        }
        return false;
    }

    bool isRunning() const { return running_; }
    bool isInitialized() const { return initialized_; }


private:
    CameraManager() : initialized_(false), mqtt_(nullptr), running_(false) {}
    ~CameraManager() { shutdown(); }
    
    CameraManager(const CameraManager&) = delete;
    CameraManager& operator=(const CameraManager&) = delete;

    std::mutex mutex_;
    cv::VideoCapture cap_;
    std::vector<int> compression_params_;
    std::vector<uchar> compressed_buffer_;
    MQTTController* mqtt_;
    bool initialized_;
    static const char* const base64_chars;
    std::atomic<bool> running_;
    std::string gstreamer_pipeline(
        int capture_width = 1280, 
        int capture_height = 720,
        int display_width = 224, 
        int display_height = 224,
        int framerate = 30, 
        int flip_method = 0) {
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
};

const char* const CameraManager::base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";