#include <iostream>
#include <vector>
#include <cassert>
// #include "./src/jpeg_header_parser.h"
struct ImageData {
    int width = 0;   // 图像宽度
    int height = 0;  // 图像高度
    int mcuWidth = 0;
    int mcuHeight = 0;
    int colorComponents = 0;
    int yQuantTableId = 0;
    int crCbQuantTableId = 1;

    // 颜色分量：Y、Cr 和 Cb 的系数块（每个块包含 64 个系数）
    std::vector<std::vector<int>> Y;   // Y 分量
    std::vector<std::vector<int>> Cr;  // Cr 分量
    std::vector<std::vector<int>> Cb;  // Cb 分量

    // 新增的二维向量，用于存储8x8块格式
    std::vector<std::vector<std::vector<int>>> Y_blocks_2D;   // Y 分量的二维 8x8 块
    std::vector<std::vector<std::vector<int>>> Cr_blocks_2D;  // Cr 分量的二维 8x8 块
    std::vector<std::vector<std::vector<int>>> Cb_blocks_2D;  // Cb 分量的二维 8x8 块

    int totalBlocks = 0;         // MCU 的总数量
    int totalYBlocks = 0;        // Y 分量块总数
    int totalCrCbBlocks = 0;     // Cr 和 Cb 分量块总数

    std::vector<uint8_t> compressedData;  // 用于存储比特流数据

    // 哈夫曼表 ID：每个分量使用的 DC 和 AC 哈夫曼表
    std::vector<int> dcTableIds = {0, 1, 1};  // 默认: Y 用表 0，Cr 和 Cb 用表 1
    std::vector<int> acTableIds = {0, 1, 1};  // 默认: Y 用表 0，Cr 和 Cb 用表 1

    // 初始化方法：根据图像宽度和高度自动计算 MCU 总数并设置块数量
    void initializeBlocks(int imageWidth, int imageHeight) {
        this->width = imageWidth;
        this->height = imageHeight;

        // 每个 MCU 覆盖 16x16 像素，计算 MCU 的数量
        mcuWidth = (width + 15) / 16;   // 向上取整，保证覆盖整个宽度
        mcuHeight = (height + 15) / 16; // 向上取整，保证覆盖整个高度
        totalBlocks = mcuWidth * mcuHeight; // MCU 总数量

        totalYBlocks = totalBlocks * 4;          // 每个 MCU 包含 4 个 Y 块
        totalCrCbBlocks = totalBlocks;           // 每个 MCU 包含 1 个 Cr 和 1 个 Cb 块

        // 初始化 Y、Cr、Cb 一维数据存储结构
        Y.resize(totalYBlocks, std::vector<int>(64, 0));     // 每个 Y 块包含 64 个系数
        Cr.resize(totalCrCbBlocks, std::vector<int>(64, 0)); // 每个 Cr 块包含 64 个系数
        Cb.resize(totalCrCbBlocks, std::vector<int>(64, 0)); // 每个 Cb 块包含 64 个系数

        // 初始化 Y、Cr、Cb 的二维 8x8 块结构
        Y_blocks_2D.resize(totalYBlocks, std::vector<std::vector<int>>(8, std::vector<int>(8, 0)));
        Cr_blocks_2D.resize(totalCrCbBlocks, std::vector<std::vector<int>>(8, std::vector<int>(8, 0)));
        Cb_blocks_2D.resize(totalCrCbBlocks, std::vector<std::vector<int>>(8, std::vector<int>(8, 0)));
    }

    // 设置哈夫曼表 ID，用于各分量的哈夫曼表编号
    void setHuffmanTableIds(int yDc, int yAc, int crCbDc, int crCbAc) {
        dcTableIds[0] = yDc;      // Y 的 DC 表
        acTableIds[0] = yAc;      // Y 的 AC 表
        dcTableIds[1] = crCbDc;   // Cr 的 DC 表
        acTableIds[1] = crCbAc;   // Cr 的 AC 表
        dcTableIds[2] = crCbDc;   // Cb 的 DC 表
        acTableIds[2] = crCbAc;   // Cb 的 AC 表
    }
};

// 创建一个模拟的 ImageData 测试对象
ImageData createTestData() {
    ImageData imgData;
    imgData.totalBlocks = 1;  // 只测试一个块
    imgData.initializeBlocks(512, 512);
    // 创建一个已知的 Zigzag 顺序矩阵（1D 转为 2D）
    int zigzagInput[64] = { 
        0, 1, 2, 3, 4, 5, 6, 7, 
        8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 
        24, 25, 26, 27, 28, 29, 30, 31,
        32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55,
        56, 57, 58, 59, 60, 61, 62, 63
    };

    // 将测试数据填入 imgData 的 Y 通道
    for (int i = 0; i < 64; ++i) {
        imgData.Y[0][i] = zigzagInput[i];
    }

    return imgData;
}

void inverseZigZag(ImageData &imgData)
{
    // Zig-Zag 索引表
    const int zigzagOrder[8][8] = 
    {{0,  1,  5,  6, 14, 15, 27, 28},
    {2,  4,  7, 13, 16, 26, 29, 42},
    {3,  8, 12, 17, 25, 30, 41, 43},
    {9, 11, 18, 24, 31, 40, 44, 53},
    {10, 19, 23, 32, 39, 45, 52, 54},
    {20, 22, 33, 38, 46, 51, 55, 60},
    {21, 34, 37, 47, 50, 56, 59, 61},
    {35, 36, 48, 49, 57, 58, 62, 63}};

    for (int mcu = 0; mcu < imgData.totalBlocks; mcu++)
    {
        for (int yBlock = 0; yBlock < 4; yBlock++)
        {
            int blockIndex = mcu * 4 + yBlock;
            for (int row = 0; row < 8; row++)
            {
                for (int col = 0; col < 8; col++)
                {
                    int Y = imgData.Y[blockIndex][zigzagOrder[row][col]];
                    imgData.Y_blocks_2D[blockIndex][row][col] = Y;
                }
            }
        }

        for (int row = 0; row < 8; row++)
        {
            for (int col = 0; col < 8; col++)
            {
                imgData.Cr_blocks_2D[mcu][row][col] = imgData.Cr[mcu][zigzagOrder[row][col]];
            }
        }

        for (int row = 0; row < 8; row++)
        {
            for (int col = 0; col < 8; col++)
            {
                imgData.Cb_blocks_2D[mcu][row][col] = imgData.Cb[mcu][zigzagOrder[row][col]];
            }
        }
    }
}
// 用于验证逆 Zigzag 的正确性
bool verifyInverseZigZag(const ImageData &imgData) {
    // 期望的行优先顺序矩阵
    int expectedOutput[8][8] = {
        { 0, 1, 5, 6, 14, 15, 27, 28 },
        { 2, 4, 7, 13, 16, 26, 29, 42 },
        { 3, 8, 12, 17, 25, 30, 41, 43 },
        { 9, 11, 18, 24, 31, 40, 44, 53 },
        { 10, 19, 23, 32, 39, 45, 52, 54 },
        { 20, 22, 33, 38, 46, 51, 55, 60 },
        { 21, 34, 37, 47, 50, 56, 59, 61 },
        { 35, 36, 48, 49, 57, 58, 62, 63 }
    };

    // 比较 imgData 中的 Y_blocks_2D 与预期输出
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (imgData.Y_blocks_2D[0][row][col] != expectedOutput[row][col]) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    // 创建测试数据
    ImageData imgData = createTestData();

    // 调用逆 Zigzag 函数
    inverseZigZag(imgData);

    // 验证结果
    if (verifyInverseZigZag(imgData)) {
        std::cout << "Zigzag 测试通过！" << std::endl;
    } else {
        std::cerr << "Zigzag 测试失败！" << std::endl;
    }

    return 0;
}
