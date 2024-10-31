#include "huffman_decoder.h"
#include <iostream>
#include <bitset>
#include <algorithm>
#include <cstdlib>
#include <unordered_map>

// 查找哈夫曼符号
int decodeHuffmanDC(BitStreamReader &reader, const HuffmanTable &dcTable) {
    int code = 0;
    std::vector<int> codeHistory;  // 记录每一步的 code 值

    // 尝试匹配哈夫曼码
    for (int length = 1; length <= 16; ++length) {
        int bit = reader.readBit();
        if (bit == -1) {
            std::cerr << "DC 解码错误：读取比特流失败。" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        code = (code << 1) | bit;  // 累积构建当前码字
        codeHistory.push_back(code);  // 记录当前 code 值

        // 在当前长度的哈夫曼表中查找匹配的码字
        auto lengthMap = dcTable.huffmanCodesByLength.find(length);
        if (lengthMap != dcTable.huffmanCodesByLength.end()) {
            auto it = lengthMap->second.find(code);
            if (it != lengthMap->second.end()) {
                int symbol = it->second;  // 获取匹配到的符号
                // std::cout << "匹配成功: Code = " << code << ", Symbol = " << symbol << std::endl;

                if (symbol == 0) return 0;  // symbol 为 0，DC 值也是 0

                // 根据 symbol 值读取附加的比特位数
                int additionalBits = reader.readBits(symbol);
                if (additionalBits < (1 << (symbol - 1))) {
                    additionalBits -= (1 << symbol) - 1;  // 将其转换为负值
                }
                return additionalBits;  // 返回实际 DC 值
            }
        }
    }

    // 如果未找到有效符号，回退指针并输出每一步的 code
    std::cerr << "DC 解码错误：未找到有效符号。" << std::endl;
    std::cerr << "解码过程中的 code 值：" << std::endl;
    for (size_t i = 0; i < codeHistory.size(); ++i) {
        std::cerr << "Step " << (i + 1) << ": Code = " << codeHistory[i] << std::endl;
    }

    std::exit(EXIT_FAILURE);
}


void decodeHuffmanAC(BitStreamReader &reader, const HuffmanTable &acTable, int *block) {
    int index = 1;  // AC 系数从索引 1 开始，因为 0 是 DC 系数

    while (index < 64) {
        int code = 0;

        // 逐位累积并匹配码字
        for (int length = 1; length <= 16; ++length) {
            int bit = reader.readBit();
            if (bit == -1) {
                std::cerr << "AC 解码错误：读取比特流失败。" << std::endl;
                std::exit(EXIT_FAILURE);
            }

            code = (code << 1) | bit;  // 累积构建当前码字

            // 在当前长度的哈夫曼表中查找匹配的码字
            auto lengthMap = acTable.huffmanCodesByLength.find(length);
            if (lengthMap != acTable.huffmanCodesByLength.end()) {
                auto it = lengthMap->second.find(code);
                if (it != lengthMap->second.end()) {
                    int symbol = it->second;  // 获取匹配到的符号

                    if (symbol == 0) {  // EOB 符号，填充剩余位置为 0
                        while (index < 64) block[index++] = 0;
                        return;
                    }

                    // 解析 symbol 的高 4 位为 runLength，低 4 位为 size
                    int runLength = (symbol >> 4) & 0xF;
                    int size = symbol & 0xF;

                    // 跳过指定的零游程
                    index += runLength;
                    if (index >= 64) return;  // 超出范围则结束

                    int acValue = 0;
                    if (size > 0) {
                        acValue = reader.readBits(size);  // 读取 size 位的值
                        // 如果值位于负数区间，转换为负值
                        if (acValue < (1 << (size - 1))) {
                            acValue -= (1 << size) - 1;
                        }
                    }

                    block[index++] = acValue;  // 将解码后的 AC 值放入块中
                    break;
                }
            }
        }
    }
}

// huffmanDecode 整体实现
void huffmanDecode(const std::vector<uint8_t> &compressedData, ImageData &imgData) {
    BitStreamReader reader(compressedData);
    // 对于每个 MCU 单元，逐块解码，暂时不做累加
    int previousDcY = 0, previousDcCr = 0, previousDcCb = 0;
    for (int mcu = 0; mcu < imgData.totalBlocks; ++mcu) {
        // 解码 4 个 Y 块
        for (int yBlock = 0; yBlock < 4; ++yBlock) {
            int blockIndex = mcu * 4 + yBlock;
            // 获取 DC 和 AC 哈夫曼表
            const HuffmanTable* dcTableY = imgData.getHuffmanTable(0, imgData.dcTableIds[0]);
            const HuffmanTable* acTableY = imgData.getHuffmanTable(1, imgData.acTableIds[0]);

            if (!dcTableY || !acTableY) {
                std::cerr << "Error: Huffman table for Y component not found." << std::endl;
                return;
            }

            // 解码 DC 系数并暂存差分值
            int dcCoefficient = decodeHuffmanDC(reader, *dcTableY);
            imgData.Y[blockIndex][0] = dcCoefficient + previousDcY; // 暂存差分值，不进行累加
            previousDcY = imgData.Y[blockIndex][0];
            // 解码 AC 系数并填充到块中
            decodeHuffmanAC(reader, *acTableY, &imgData.Y[blockIndex][0]);
        }

        // 解码 Cr 块
        const HuffmanTable* dcTableCr = imgData.getHuffmanTable(0, imgData.dcTableIds[1]);
        const HuffmanTable* acTableCr = imgData.getHuffmanTable(1, imgData.acTableIds[1]);

        if (!dcTableCr || !acTableCr) {
            std::cerr << "Error: Huffman table for Cr component not found." << std::endl;
            return;
        }

        // 解码 Cr 的 DC 系数并暂存差分值
        int dcCoefficientCr = decodeHuffmanDC(reader, *dcTableCr);
        imgData.Cr[mcu][0] = dcCoefficientCr + previousDcCr; // 暂存差分值
        previousDcCr = imgData.Cr[mcu][0];
        // 解码 Cr 的 AC 系数并填充到块中
        decodeHuffmanAC(reader, *acTableCr, &imgData.Cr[mcu][0]);

        // 解码 Cb 块
        const HuffmanTable* dcTableCb = imgData.getHuffmanTable(0, imgData.dcTableIds[2]);
        const HuffmanTable* acTableCb = imgData.getHuffmanTable(1, imgData.acTableIds[2]);

        if (!dcTableCb || !acTableCb) {
            std::cerr << "Error: Huffman table for Cb component not found." << std::endl;
            return;
        }

        // 解码 Cb 的 DC 系数并暂存差分值
        int dcCoefficientCb = decodeHuffmanDC(reader, *dcTableCb);
        imgData.Cb[mcu][0] = dcCoefficientCb + previousDcCb; // 暂存差分值
        previousDcCb = imgData.Cb[mcu][0];
        // 解码 Cb 的 AC 系数并填充到块中
        decodeHuffmanAC(reader, *acTableCb, &imgData.Cb[mcu][0]);
    }

    std::cout << "Huffman Decoding ends" << std::endl;
}

