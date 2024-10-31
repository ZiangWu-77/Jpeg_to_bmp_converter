#ifndef JPEG_HEADER_PARSER_H
#define JPEG_HEADER_PARSER_H

#include <vector>
#include <fstream>
#include <map>
#include "jpeg_parser_helpers.h"

// JPEG 文件头常量
constexpr uint8_t SOI = 0xD8;   // Start of Image
constexpr uint8_t SOF0 = 0xC0;  // Start of Frame
constexpr uint8_t DQT = 0xDB;   // Define Quantization Table
constexpr uint8_t DHT = 0xC4;   // Define Huffman Table
constexpr uint8_t SOS = 0xDA;   // Start of Scan
constexpr uint8_t EOI = 0xD9;   // End of Image
constexpr uint8_t DRI = 0xDD;

// Huffman Table
#include <unordered_map>

struct HuffmanTable {
    int tableClass;                // 0 表示 DC 表，1 表示 AC 表
    int tableId;                   // 表 ID
    std::vector<int> lengths;      // 符号长度数组（每个长度对应的符号数量）
    std::vector<int> symbols;      // 符号数组
    std::unordered_map<int, std::unordered_map<int, int>> huffmanCodesByLength;  // 按长度存储的哈夫曼码表

    void buildHuffmanCodes() {
        int code = 0;
        int currentLength = 0;
        int symbolIndex = 0;
        huffmanCodesByLength.clear();

        for (int length = 1; length <= 16; ++length) {
            int numSymbols = lengths[length - 1];

            if (numSymbols > 0) {
                // 在进入新长度时，左移 code
                if (currentLength != length) {
                    code <<= (length - currentLength);
                    currentLength = length;
                }

                for (int i = 0; i < numSymbols; ++i) {
                    huffmanCodesByLength[length][code] = symbols[symbolIndex++];
                    code++;  // 增加 code，为下一个符号准备
                }
            }
        }
    }
};

struct ImageData {
    int width = 0;   // 图像宽度
    int height = 0;  // 图像高度
    int mcuWidth = 0;
    int mcuHeight = 0;
    int colorComponents = 0;
    int yQuantTableId = 0;
    int crCbQuantTableId = 1;
    std::map<int, std::vector<int>> quantizationTables;  // 量化表
    std::vector<HuffmanTable> huffmanTables;           // 哈夫曼表

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
    
    // 查找哈夫曼表的方法，依据表类型（0 表示 DC，1 表示 AC）和表 ID
    const HuffmanTable* getHuffmanTable(int tableType, int tableId) const {
        for (const auto& table : huffmanTables) {
            if (table.tableClass == tableType && table.tableId == tableId) {
                return &table;
            }
        }
        return nullptr;  // 如果没有找到对应表，返回空指针
    }

    // 初始化所有哈夫曼表，预先计算每个哈夫曼表的哈夫曼码表
    void initializeHuffmanTables() {
        for (auto& table : huffmanTables) {
            table.buildHuffmanCodes();  // 每个哈夫曼表构建自己的码表
        }
    }

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

// BitStreamReader 类用于逐位读取压缩数据流
class BitStreamReader {
public:
    explicit BitStreamReader(const std::vector<uint8_t> &data);
    int readBits(int numBits);     // 读取指定数量的比特
    int readBit();                 // 读取一个比特

private:
    const std::vector<uint8_t> &data; // 数据流引用
    size_t bytePos;                    // 当前字节位置
    int bitPos;                        // 当前字节中的比特位置
};

// 解析 JPEG 文件头
ImageData parseJPEGHeader(const std::string &filename);

#endif // JPEG_HEADER_PARSER_H