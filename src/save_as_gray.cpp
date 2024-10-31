#include <iostream>
#include <opencv2/opencv.hpp> // OpenCV 头文件
#include "jpeg_header_parser.h"  // 假设定义了 ImageData 结构体

template <typename T>
T clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

void mapBlocksToGrayImage(const ImageData &imgData, cv::Mat &colorImage) {
    int width = imgData.width;
    int height = imgData.height;
    int mcuPerRow = width / 16; // 每行的 MCU 数量（宽度 / 16）
    int mcuPerCol = height / 16;

    // 遍历每个 Y 块，将其值填充到灰度图像中
    for (int block = 0; block < imgData.totalYBlocks; block++)
    {
        int strow = 0, stcol = 0;
        if (block % 4 == 0) 
        {
            strow = (block / 4 / mcuPerRow) * 16;
            stcol = (block / 4 % mcuPerRow) * 16;
        }
        else if (block % 4 == 1)
        {
            strow = (block / 4 / mcuPerRow) * 16;
            stcol = (block / 4 % mcuPerRow) * 16 + 8;
        }
        else if (block % 4 == 2)
        {
            strow = (block / 4 / mcuPerRow) * 16 + 8;
            stcol = (block / 4 % mcuPerRow) * 16;
        }
        else if (block % 4 == 3)
        {
            strow = (block / 4 / mcuPerRow) * 16 + 8;
            stcol = (block / 4 % mcuPerRow) * 16 + 8;
        }

        for (int row = 0; row < 8; row++)
        {
            for (int col = 0; col < 8; col++)
            {
                int Crrow = 0, Crcol=0;
                if (block % 4 == 0) 
                {
                    Crrow = row / 2;
                    Crcol = col / 2;
                }
                if (block % 4 == 1)
                {
                    Crrow = row / 2;
                    Crcol = 4 + col / 2;
                }
                if (block % 4 == 2)
                {
                    Crrow = 4 + row / 2;
                    Crcol = col / 2;
                }
                if (block % 4 == 3)
                {
                    Crrow = 4 + row / 2;
                    Crcol = col / 2 + 4;
                }
                int Y = imgData.Y_blocks_2D[block][row][col];
                int Cr = imgData.Cr_blocks_2D[block/4][Crrow][Crcol];
                int Cb = imgData.Cb_blocks_2D[block/4][Crrow][Crcol];
                Y = static_cast<uint8_t>(clamp(Y + 128, 0, 255));
                Cr = static_cast<uint8_t>(clamp(Cr + 128, 0, 255));
                Cb = static_cast<uint8_t>(clamp(Cb + 128, 0, 255));
                int R = Y + 1.402 * (Cr - 128);
                int G = Y - 0.344136 * (Cb - 128) - 0.714136 * (Cr - 128);
                int B = Y + 1.772 * (Cb - 128);
                R = static_cast<uint8_t>(clamp(R, 0, 255));
                G = static_cast<uint8_t>(clamp(G, 0, 255));
                B = static_cast<uint8_t>(clamp(B, 0, 255));
                int tmprow = strow + row;
                int tmpcol = stcol + col;
                // 将 Y 值填充到灰度图像中
                colorImage.at<cv::Vec3b>(tmprow, tmpcol) = cv::Vec3b(R, G, B);
            }
        }
    }
}

void saveAsImage(const std::string &filename, const ImageData &imgData) {
    int width = imgData.width;
    int height = imgData.height;

    // 创建一个 Mat 对象，用于存储灰度图像
    cv::Mat colorImage(height, width, CV_8UC3, cv::Scalar(0, 0, 0));

    // 将 Y 分量填充到灰度图像中
    mapBlocksToGrayImage(imgData, colorImage);

    // 将图像保存为 PNG 或 BMP 格式
    if (!cv::imwrite(filename, colorImage)) {
        std::cerr << "无法创建图像文件: " << filename << std::endl;
    }
}
