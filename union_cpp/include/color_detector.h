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
    const cv::Scalar yellowLower{20, 100, 100};
    const cv::Scalar yellowUpper{30, 255, 255};
    const cv::Scalar purpleLower{130, 50, 50};
    const cv::Scalar purpleUpper{160, 255, 255};
    
    bool detectSpecificColor(const cv::Mat& frame, const cv::Scalar& lower, const cv::Scalar& upper);
};
