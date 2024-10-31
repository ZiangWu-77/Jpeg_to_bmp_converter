#include "jpeg_header_parser.h"
#include <iostream>
#include <fstream>
#include <vector>

// JPEG 解析函数
ImageData parseJPEGHeader(const std::string &filename) {
    ImageData imgData;
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return imgData;
    }

    // 检查起始标记 (SOI)
    if (file.get() != 0xFF || file.get() != SOI) {
        std::cerr << "不是有效的 JPEG 文件" << std::endl;
        return imgData;
    }

    while (file) {
        if (file.get() != 0xFF) {
            std::cerr << "JPEG 格式错误: 缺少标记" << std::endl;
            break;
        }

        uint8_t marker = file.get();
        uint16_t length = readBigEndian16(file) - 2;
        if (marker == DRI) break;
        if (marker == SOF0) {
            file.get(); // 忽略精度
            imgData.height = readBigEndian16(file);
            imgData.width = readBigEndian16(file);
            
            // 获取颜色分量数
            imgData.colorComponents = file.get(); // 颜色分量数
            std::cout << "Number of Color Components: " << static_cast<int>(imgData.colorComponents) << std::endl;
            
            // 解析每个颜色分量的信息
            for (int i = 0; i < imgData.colorComponents; ++i) {
                uint8_t componentID = file.get();        // 颜色分量 ID（如 1 = Y, 2 = Cr, 3 = Cb）
                uint8_t samplingFactors = file.get();    // 水平和垂直采样因子
                uint8_t quantizationTableID = file.get();// 量化表 ID

                // 提取水平和垂直采样因子
                int horizontalSamplingFactor = (samplingFactors >> 4) & 0xF;
                int verticalSamplingFactor = samplingFactors & 0xF;

                // 输出颜色分量的信息
                std::cout << "Component " << static_cast<int>(componentID) << ":\n";
                std::cout << "  Horizontal Sampling Factor: " << horizontalSamplingFactor << std::endl;
                std::cout << "  Vertical Sampling Factor: " << verticalSamplingFactor << std::endl;
                std::cout << "  Quantization Table ID: " << static_cast<int>(quantizationTableID) << std::endl;

                // 打印该分量对应的量化表内容
                const std::vector<int> &quantTable = imgData.quantizationTables[quantizationTableID];
                std::cout << "  Quantization Table Values: ";
                for (int q : quantTable) {
                    std::cout << q << " ";
                }
                std::cout << std::endl;
            }
        } else if (marker == DQT) {
            // 读取量化表
            while (length > 0) {
                uint8_t precisionAndTableId = file.get();
                int tableId = precisionAndTableId & 0x0F;  // 获取量化表 ID
                std::cout << "TableID" << " " << tableId << std::endl;
                int precision = (precisionAndTableId >> 4) ? 16 : 8; // 获取精度
                std::vector<int> quantTable(64);

                for (int i = 0; i < 64; i++) {
                    quantTable[i] = (precision == 8) ? file.get() : readBigEndian16(file);
                }
                
                // 根据 tableId 保存量化表
                imgData.quantizationTables[tableId] = quantTable;
                
                // 减少剩余长度
                length -= (precision == 8) ? 65 : 129;
            }
        } else if (marker == DHT) {
            // 读取哈夫曼表
            std::cout << "huffman data length: " << length << std::endl; 
            HuffmanTable huffTable;
            uint8_t tableClassAndId = file.get();
            huffTable.tableClass = (tableClassAndId >> 4);
            huffTable.tableId = tableClassAndId & 0x0F;

            huffTable.lengths.resize(16);
            for (int i = 0; i < 16; ++i) {
                huffTable.lengths[i] = file.get();
            }

            int totalSymbols = 0;
            for (int len : huffTable.lengths) {
                totalSymbols += len;
            }

            huffTable.symbols.resize(totalSymbols);
            for (int i = 0; i < totalSymbols; ++i) {
                huffTable.symbols[i] = file.get();
            }
            imgData.huffmanTables.push_back(huffTable);
        } else if (marker == SOS) {
            // 提取 SOS 段比特流数据
            uint8_t byte;
            while (length > 0) {
                file.get();
                length -= 1;
            }
            while (file.read(reinterpret_cast<char*>(&byte), 1)) {
                if (byte == 0xFF) {
                    // 读取下一个字节
                    uint8_t nextByte = file.get();
                    
                    if (nextByte == 0x00) {
                        // 情况 1：0xFF 0x00，表示实际数据的 0xFF
                        imgData.compressedData.push_back(0xFF);
                    } else if (nextByte == 0xD9) {
                        // 情况 2：0xFF 0xD9，表示 EOI 结束标记
                        break;
                    }
                    else if (nextByte >= 0xD0 && nextByte <= 0xD7) {
                        // 情况 3：0xFF 0xD0 ~ 0xFF 0xD7 表示复位标记 RSTn，忽略它
                        // 跳过并继续处理下一个字节
                        // std::cout << "there is a RSTn!" << std::endl;
                        // break;
                        continue;
                    }
                    // else if (nextByte == 0xFF) {
                    //     // 情况 4：连续的 0xFF，忽略当前的 0xFF，重新检查下一个 0xFF
                    //     file.seekg(-1, std::ios::cur); // 回退文件指针一个字节
                    // }
                    else {
                        // 情况 5：其他情况，保留 0xFF 和随后的字节
                        // imgData.compressedData.push_back(0xFF);
                        imgData.compressedData.push_back(nextByte);
                        break;
                    }
                } else {
                    // 正常数据字节
                    imgData.compressedData.push_back(byte);
                }
            }
            break;
        } else {
            file.ignore(length);
        }
    }

    file.close();
    return imgData;
}

// BitStreamReader 构造函数和方法
BitStreamReader::BitStreamReader(const std::vector<uint8_t> &data) : data(data), bytePos(0), bitPos(0) {}
int BitStreamReader::readBit() {
    // 检查当前字节位置是否越界
    if (bytePos >= data.size()) {
        std::cerr << "Reached end of data while reading bits." << std::endl;
        return -1;  // 超出数据范围，返回错误码
    }

    // 提取当前字节的第 bitPos 位，位移并进行位掩码
    int bit = (data[bytePos] >> (7 - bitPos)) & 1;
    bitPos++;

    // 如果 bitPos 达到 8，重置并移动到下一个字节
    if (bitPos == 8) {
        bitPos = 0;
        bytePos++;
    }

    // 输出当前读取的位和字节位置信息，用于调试
    // std::cout << "Read bit: " << bit << " at byte position: " << bytePos << ", bit position: " << bitPos << std::endl;
    return bit;
}

int BitStreamReader::readBits(int numBits) {
    int value = 0;
    while (numBits > 0) {
        int bit = readBit();
        if (bit == -1) {
            std::cerr << "Error reading bit stream." << std::endl;
            return -1;
        }
        value = (value << 1) | bit;
        --numBits;
    }

    // Debug output to verify each read
    // std::cout << "Read bits: " << value << " with requested bits" << std::endl;
    return value;
}

