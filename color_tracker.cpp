#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>
#include <algorithm>
#include "Servos.h"

class ColorTracker {
private:
    // Color tracking parameters  
    cv::Scalar colorLower;  // HSV lower bound
    cv::Scalar colorUpper;  // HSV upper bound
    const int ERROR_TOLERANCE = 30;  // Position tolerance
    const float PID_P = 1.0f;  // Drastically reduced for very slow movement
    const int FRAME_CENTER_X = 150;
    const int FRAME_CENTER_Y = 150;
    
    // Servo configuration
    Servos servos;
    const int SERVO_PAN = 1;   // Horizontal movement servo (ID 1)
    const int SERVO_TILT = 5;  // Vertical movement servo (ID 5)
    
    // Servo angle limits
    const int PAN_LEFT_LIMIT = -80;    // Maximum left rotation
    const int PAN_RIGHT_LIMIT = 80;    // Maximum right rotation
    const int TILT_DOWN_LIMIT = -80;   // Maximum downward tilt
    const int TILT_UP_LIMIT = 45;      // Maximum upward tilt
    
    // Current servo positions
    float currentPanAngle = 0.0f;
    float currentTiltAngle = 0.0f;
    
    // Movement smoothing
    const float MAX_ANGLE_CHANGE = 0.8f;  // Maximum 1 degree change per frame
    
    // Helper function to limit angle within range
 float limitAngle(float angle, int minAngle, int maxAngle) {
        if(angle > maxAngle)
            return maxAngle;
        else if (angle < minAngle)
            return minAngle;
        else
            return angle;
    }
    
    // Helper function to limit movement speed
    float limitMovement(float current, float target) {
        float diff = target - current;
        if (std::abs(diff) > MAX_ANGLE_CHANGE) {
            if (diff > 0) {
                return current + MAX_ANGLE_CHANGE;
            } else {
                return current - MAX_ANGLE_CHANGE;
            }
        }
        return target;
    }
    
void moveServo(int servoId, float targetAngle) {
        float limitedAngle;
        if (servoId == SERVO_PAN) {
            //targetAngle = -targetAngle;  // Reverse the direction
            limitedAngle = limitAngle(targetAngle, PAN_LEFT_LIMIT, PAN_RIGHT_LIMIT);
            limitedAngle = limitMovement(currentPanAngle, limitedAngle);
            currentPanAngle = limitedAngle;
        } else {  // SERVO_TILT
            //targetAngle = -targetAngle;
            limitedAngle = limitAngle(targetAngle, TILT_DOWN_LIMIT, TILT_UP_LIMIT);
            limitedAngle = limitMovement(currentTiltAngle, limitedAngle);
            currentTiltAngle = limitedAngle;
        }
        
        // Convert float angle to int with proper rounding
        int servoAngle = static_cast<int>(std::round(limitedAngle * 10.0f)) * 0.1f;
        servos.setServoAngle(servoId, servoAngle);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));  // Reduced delay for smoother movement
    }
    
    
    void adjustPanAngle(int error) {
        float adjustment = error * PID_P;
        float newAngle = currentPanAngle + adjustment;
        moveServo(SERVO_PAN, newAngle);
    }
    
    void adjustTiltAngle(int error) {
        float adjustment = error * PID_P;
        float newAngle = currentTiltAngle + adjustment;
        moveServo(SERVO_TILT, newAngle);
    }

public:
    ColorTracker() : servos("/dev/ttyTHS1", 1000000) {
        // Initialize with more strict yellow color bounds
        colorLower = cv::Scalar(25, 150, 150);  // Higher saturation and value for stronger yellow
        colorUpper = cv::Scalar(35, 255, 255);  // Narrower hue range
        
        // Initialize servos to center position (0 degrees)
        moveServo(SERVO_PAN, 0);
        moveServo(SERVO_TILT, 0);
    }
    
    void processFrame(cv::Mat& frame) {
        cv::Mat hsv;
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        
        cv::Mat mask;
        cv::inRange(hsv, colorLower, colorUpper, mask);
        
        cv::erode(mask, mask, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
        cv::dilate(mask, mask, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
        
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        if (!contours.empty()) {
            auto largest_contour = std::max_element(contours.begin(), contours.end(),
                [](const std::vector<cv::Point>& c1, const std::vector<cv::Point>& c2) {
                    return cv::contourArea(c1) < cv::contourArea(c2);
                });
                
            if (cv::contourArea(*largest_contour) > 100) {  // Minimum area threshold
                // Find the center using moments instead of minEnclosingCircle
                cv::Moments m = cv::moments(*largest_contour);
                cv::Point2f center(m.m10/m.m00, m.m01/m.m00);
                
                // Get the bounding rectangle
                cv::Rect boundRect = cv::boundingRect(*largest_contour);
                
                // Draw rectangle around target
                cv::rectangle(frame, boundRect, cv::Scalar(255, 255, 255), 2);
                
                // Draw center crosshair
                int crosshairSize = 10;
                cv::line(frame, 
                    cv::Point(center.x - crosshairSize, center.y),
                    cv::Point(center.x + crosshairSize, center.y),
                    cv::Scalar(0, 0, 255), 2);
                cv::line(frame, 
                    cv::Point(center.x, center.y - crosshairSize),
                    cv::Point(center.x, center.y + crosshairSize),
                    cv::Scalar(0, 0, 255), 2);
                
                // Calculate errors from center
                int error_X = static_cast<int>(center.x) - FRAME_CENTER_X;
                int error_Y = static_cast<int>(center.y) - FRAME_CENTER_Y;
                
                // Draw target center and frame center connection line
                cv::line(frame, 
                    cv::Point(FRAME_CENTER_X, FRAME_CENTER_Y),
                    cv::Point(center.x, center.y),
                    cv::Scalar(0, 255, 0), 1);
                
                // Show error distances on screen
                cv::putText(frame, 
                    "X Error: " + std::to_string(error_X), 
                    cv::Point(10, 110),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
                cv::putText(frame, 
                    "Y Error: " + std::to_string(error_Y), 
                    cv::Point(10, 130),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
                
                // Move servos if error is significant
                if (std::abs(error_Y) > ERROR_TOLERANCE) {
                    adjustTiltAngle(error_Y);
                    std::string direction = error_Y < 0 ? "Up" : "Down";
                    cv::putText(frame, "Looking " + direction, cv::Point(10, 50),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
                    
                    // Display current tilt angle
                     cv::putText(frame, 
            "Pan Angle: " + std::to_string(currentPanAngle).substr(0, 5),
            cv::Point(10, 170),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
                } else {
                    cv::putText(frame, "Y Axis Locked", cv::Point(10, 50),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
                }
                
                if (std::abs(error_X) > ERROR_TOLERANCE) {
                    adjustPanAngle(error_X);
                    std::string direction = error_X < 0 ? "Left" : "Right";
                    cv::putText(frame, "Looking " + direction, cv::Point(10, 80),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
                    
                    // Display current pan angle
                    cv::putText(frame, 
            "Tilt Angle: " + std::to_string(currentTiltAngle).substr(0, 5),
            cv::Point(10, 150),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
                } else {
                    cv::putText(frame, "X Axis Locked", cv::Point(10, 80),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
                }
                
                cv::putText(frame, "Target Detected", cv::Point(10, 20),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
            }
        } else {
            cv::putText(frame, "Target Detecting", cv::Point(10, 20),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
        }
        
        // Draw frame center crosshair
        int centerCrosshairSize = 20;
        cv::line(frame, 
            cv::Point(FRAME_CENTER_X - centerCrosshairSize, FRAME_CENTER_Y),
            cv::Point(FRAME_CENTER_X + centerCrosshairSize, FRAME_CENTER_Y),
            cv::Scalar(0, 255, 0), 2);
        cv::line(frame, 
            cv::Point(FRAME_CENTER_X, FRAME_CENTER_Y - centerCrosshairSize),
            cv::Point(FRAME_CENTER_X, FRAME_CENTER_Y + centerCrosshairSize),
            cv::Scalar(0, 255, 0), 2);
    }
};
int main() {
    // GStreamer pipeline for CSI camera
    std::string gstreamer_pipeline = "nvarguscamerasrc ! "
        "video/x-raw(memory:NVMM), "
        "width=(int)300, height=(int)300, "
        "format=(string)NV12, framerate=(fraction)30/1 ! "
        "nvvidconv ! "
        "video/x-raw, format=(string)BGRx ! "
        "videoconvert ! "
        "video/x-raw, format=(string)BGR ! "
        "appsink";
    
    cv::VideoCapture cap(gstreamer_pipeline, cv::CAP_GSTREAMER);
    
    if (!cap.isOpened()) {
        std::cerr << "Failed to open camera!" << std::endl;
        return -1;
    }
    
    ColorTracker tracker;
    cv::Mat frame;
    
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: Could not read frame." << std::endl;
            break;
        }
        
        tracker.processFrame(frame);
        cv::imshow("Color Tracking", frame);
        
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }
    
    cap.release();
    cv::destroyAllWindows();
    return 0;
}