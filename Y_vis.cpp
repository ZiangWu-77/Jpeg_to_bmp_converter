#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // 读取 RGB 图像
    cv::Mat rgbImage = cv::imread("./input/lena.jpg");
    if (rgbImage.empty()) {
        std::cerr << "Error: Could not open the image!" << std::endl;
        return -1;
    }

    // 将 RGB 图像转换为 YCbCr 颜色空间
    cv::Mat ycbcrImage;
    cv::cvtColor(rgbImage, ycbcrImage, cv::COLOR_BGR2YCrCb);

    // 分离 Y, Cb, Cr 通道
    std::vector<cv::Mat> channels(3);
    cv::split(ycbcrImage, channels);
    cv::Mat yChannel = channels[0]; // Y 通道

    // 可视化 Y 通道
    // cv::namedWindow("Y Channel", cv::WINDOW_NORMAL);
    // cv::imshow("Y Channel", yChannel);

    // 保存 Y 通道为 BMP 图像
    cv::imwrite("Y_channel.bmp", yChannel);

    // 等待按键关闭
    cv::waitKey(0);
    return 0;
}
