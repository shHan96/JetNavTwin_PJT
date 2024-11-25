#include "color_detector.h"

ColorDetector::ColorDetector(TargetColor initialColor) : currentTarget(initialColor) {}

bool ColorDetector::detectColor(const cv::Mat& frame) {
    // ROI 설정 - 화면 하단 부분만 선택
    int roiHeight = frame.rows * (1 - ROI_START_RATIO);
    cv::Mat roi = frame(cv::Rect(0, frame.rows - roiHeight, frame.cols, roiHeight));
    
    // HSV 변환
    cv::Mat hsv;
    cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);
    
    // 노이즈 제거를 위한 블러 처리
    cv::GaussianBlur(hsv, hsv, cv::Size(5, 5), 0);
    
    if (currentTarget == TargetColor::YELLOW) {
        return detectSpecificColor(hsv, yellowLower, yellowUpper);
    } else {
        return detectSpecificColor(hsv, purpleLower, purpleUpper);
    }
}




bool ColorDetector::detectSpecificColor(const cv::Mat& frame, const cv::Scalar& lower, const cv::Scalar& upper) {
    cv::Mat mask;
    cv::inRange(frame, lower, upper, mask);
    
    // 노이즈 제거를 위한 모폴로지 연산
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::erode(mask, mask, kernel);
    cv::dilate(mask, mask, kernel);
    
    int totalPixels = frame.rows * frame.cols;
    int coloredPixels = cv::countNonZero(mask);
    
    // 임계값을 약간 낮춤 (ROI를 사용하므로 더 작은 영역도 감지 가능)
    float threshold = (currentTarget == TargetColor::PURPLE) ? 0.03f : 0.05f;
    
    return (static_cast<float>(coloredPixels) / totalPixels) > threshold;
}

void ColorDetector::setTargetColor(TargetColor color) {
    currentTarget = color;
}