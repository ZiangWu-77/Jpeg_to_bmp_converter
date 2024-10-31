#include "save_as_bmp.h"
#include <fstream>
#include <vector>
#include <iostream>

template <typename T>
T clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

bool saveAsBMP(const std::string &filename, const ImageData &imgData) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "无法创建 BMP 文件: " << filename << std::endl;
        return false;
    }

    int width = imgData.width;
    int height = imgData.height;
    int mcuPerRow = width / 16; // 每行的 MCU 数量
    int rowSize = ((width * 3 + 3) / 4) * 4; // 每行按 4 字节对齐
    int dataSize = rowSize * height;
    int fileSize = 54 + dataSize;

    // BMP 文件头
    uint8_t fileHeader[14] = {
        'B', 'M',                          // 文件类型
        static_cast<uint8_t>(fileSize), static_cast<uint8_t>(fileSize >> 8), static_cast<uint8_t>(fileSize >> 16), static_cast<uint8_t>(fileSize >> 24), // 文件大小
        0, 0, 0, 0,                        // 保留字段
        54, 0, 0, 0                        // 像素数据偏移量
    };
    file.write(reinterpret_cast<char*>(fileHeader), sizeof(fileHeader));

    // BMP 信息头
    uint8_t infoHeader[40] = {
        40, 0, 0, 0,                       // 信息头大小
        static_cast<uint8_t>(width), static_cast<uint8_t>(width >> 8), static_cast<uint8_t>(width >> 16), static_cast<uint8_t>(width >> 24), // 宽度
        static_cast<uint8_t>(height), static_cast<uint8_t>(height >> 8), static_cast<uint8_t>(height >> 16), static_cast<uint8_t>(height >> 24), // 高度
        1, 0,                              // 色平面数
        24, 0,                             // 位深 (24 位)
        0, 0, 0, 0,                        // 无压缩
        static_cast<uint8_t>(dataSize), static_cast<uint8_t>(dataSize >> 8), static_cast<uint8_t>(dataSize >> 16), static_cast<uint8_t>(dataSize >> 24), // 图像数据大小
        0, 0, 0, 0,                        // 水平分辨率
        0, 0, 0, 0,                        // 垂直分辨率
        0, 0, 0, 0                         // 色彩数
    };
    file.write(reinterpret_cast<char*>(infoHeader), sizeof(infoHeader));

    // 创建缓冲区来存储像素数据
    std::vector<uint8_t> pixelData(dataSize, 0);

    // 遍历每个 Y 块，将其值填充到像素数据缓冲区中
    for (int block = 0; block < imgData.totalYBlocks; block++) {
        int strow = 0, stcol = 0;
        if (block % 4 == 0) {
            strow = (block / 4 / mcuPerRow) * 16;
            stcol = (block / 4 % mcuPerRow) * 16;
        } else if (block % 4 == 1) {
            strow = (block / 4 / mcuPerRow) * 16;
            stcol = (block / 4 % mcuPerRow) * 16 + 8;
        } else if (block % 4 == 2) {
            strow = (block / 4 / mcuPerRow) * 16 + 8;
            stcol = (block / 4 % mcuPerRow) * 16;
        } else if (block % 4 == 3) {
            strow = (block / 4 / mcuPerRow) * 16 + 8;
            stcol = (block / 4 % mcuPerRow) * 16 + 8;
        }

        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                int Crrow = 0, Crcol = 0;
                if (block % 4 == 0) {
                    Crrow = row / 2;
                    Crcol = col / 2;
                } else if (block % 4 == 1) {
                    Crrow = row / 2;
                    Crcol = 4 + col / 2;
                } else if (block % 4 == 2) {
                    Crrow = 4 + row / 2;
                    Crcol = col / 2;
                } else if (block % 4 == 3) {
                    Crrow = 4 + row / 2;
                    Crcol = 4 + col / 2;
                }

                int Y = imgData.Y_blocks_2D[block][row][col];
                int Cr = imgData.Cr_blocks_2D[block / 4][Crrow][Crcol];
                int Cb = imgData.Cb_blocks_2D[block / 4][Crrow][Crcol];

                Y = clamp(Y + 128, 0, 255);
                Cr = clamp(Cr + 128, 0, 255);
                Cb = clamp(Cb + 128, 0, 255);

                int R = clamp(int(Y + 1.402 * (Cr - 128)), 0, 255);
                int G = clamp(int(Y - 0.344136 * (Cb - 128) - 0.714136 * (Cr - 128)), 0, 255);
                int B = clamp(int(Y + 1.772 * (Cb - 128)), 0, 255);

                int tmprow = strow + row;
                int tmpcol = stcol + col;
                int pixelIndex = (height - 1 - tmprow) * rowSize + tmpcol * 3; // 从下往上存储

                pixelData[pixelIndex] = static_cast<uint8_t>(R);
                pixelData[pixelIndex + 1] = static_cast<uint8_t>(G);
                pixelData[pixelIndex + 2] = static_cast<uint8_t>(B);
            }
        }
    }

    // 写入像素数据
    file.write(reinterpret_cast<char*>(pixelData.data()), dataSize);
    file.close();
    return true;
}
