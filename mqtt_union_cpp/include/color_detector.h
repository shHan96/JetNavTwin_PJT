#pragma once
#include <opencv2/opencv.hpp>
#include <atomic>

class ColorDetector {
public:
    enum class TargetColor {
        YELLOW,
        PURPLE
    };

    ColorDetector(TargetColor initialColor);
    bool detectColor(const cv::Mat& frame);
    void setTargetColor(TargetColor color);

private:
    TargetColor currentTarget;
    // Yellow HSV 범위 (기존과 동일)
    const cv::Scalar yellowLower{20, 100, 100};
    const cv::Scalar yellowUpper{30, 255, 255};
    // 보라색 HSV 범위 조정 - 밝은 보라색도 포함하고 파란색은 제외
    const cv::Scalar purpleLower{125, 40, 40};   // H: 보라색 시작점을 약간 낮춤
    const cv::Scalar purpleUpper{150, 255, 255}; // H: 끝점을 조정하여 파란색 제외
    
    // ROI(관심 영역) 설정을 위한 비율
    const float ROI_START_RATIO = 0.95; // 화면 하단 40% 영역만 검사
    
    bool detectSpecificColor(const cv::Mat& frame, const cv::Scalar& lower, const cv::Scalar& upper);
};
