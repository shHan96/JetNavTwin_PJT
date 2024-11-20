#include "color_detector.h"

ColorDetector::ColorDetector(TargetColor initialColor) : currentTarget(initialColor) {}

bool ColorDetector::detectColor(const cv::Mat& frame) {
    cv::Mat hsv;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
    
    if (currentTarget == TargetColor::YELLOW) {
        return detectSpecificColor(hsv, yellowLower, yellowUpper);
    } else {
        return detectSpecificColor(hsv, purpleLower, purpleUpper);
    }
}



bool ColorDetector::detectSpecificColor(const cv::Mat& frame, const cv::Scalar& lower, const cv::Scalar& upper) {
    cv::Mat mask;
    cv::inRange(frame, lower, upper, mask);
    
    int totalPixels = frame.rows * frame.cols;
    int coloredPixels = cv::countNonZero(mask);
    
    // If more than 5% of pixels are the target color, consider it detected
    return (static_cast<float>(coloredPixels) / totalPixels) > 0.05;
}

void ColorDetector::setTargetColor(TargetColor color) {
    currentTarget = color;
}