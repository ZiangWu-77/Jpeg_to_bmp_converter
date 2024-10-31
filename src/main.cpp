#include "jpeg_header_parser.h"
#include "jpeg_decoder.h"
#include "jpeg_decoder.h"
#include "save_as_bmp.h"
#include "save_as_gray.h"
#include <iostream>

void saveCompressedData(const std::vector<uint8_t>& compressedData, const std::string& filename) {
    // 打开文件进行二进制写入
    std::ofstream outputFile(filename, std::ios::binary);
    
    if (!outputFile) {
        std::cerr << "无法创建文件: " << filename << std::endl;
        return;
    }

    // 将数据写入文件
    outputFile.write(reinterpret_cast<const char*>(compressedData.data()), compressedData.size());

    // 关闭文件
    outputFile.close();

    std::cout << "数据已保存到文件: " << filename << std::endl;
}

int main() {
    std::string filename = "../input/lena.jpg";
    ImageData imgData = parseJPEGHeader(filename);

    if (imgData.width && imgData.height) {
        std::cout << "图像宽度: " << imgData.width << ", 高度: " << imgData.height << std::endl;
    } else {
        std::cerr << "解析图像头部失败！" << std::endl;
        return -1;
    }

    std::cout << "量化表数量: " << imgData.quantizationTables.size() << std::endl;
    std::cout << "哈夫曼表数量: " << imgData.huffmanTables.size() << std::endl;

    imgData.initializeHuffmanTables();
    // 在 parseJPEGHeader 函数中解析完哈夫曼表后，打印长度、符号和 Huffman 码表
    for (const auto& table : imgData.huffmanTables) {
        std::cout << "Huffman Table ID: " << table.tableId << ", Class: " << table.tableClass << std::endl;

        // 打印符号长度数组
        std::cout << "Lengths: ";
        for (int len : table.lengths) {
            std::cout << len << " ";
        }
        std::cout << "\nSymbols: ";
        for (int sym : table.symbols) {
            std::cout << sym << " ";
        }
        
        // 打印哈夫曼码表
        std::cout << "\nHuffman Codes:" << std::endl;
        for (const auto& [length, codes] : table.huffmanCodesByLength) {
            for (const auto& [code, symbol] : codes) {
                std::cout << "Code: ";

                // 打印指定长度的二进制码字，补齐前导0
                for (int i = length - 1; i >= 0; --i) {
                    std::cout << ((code >> i) & 1);
                }

                std::cout << " (binary, Length: " << length << "), Symbol: " << symbol << std::endl;
            }
        }
        std::cout << std::endl;
    }

    saveCompressedData(imgData.compressedData, "../input/sos_compressed_data.bin");    
    
    // 初始化图像数据块结构
    imgData.initializeBlocks(imgData.width, imgData.width);
    std::cout << imgData.totalBlocks << std::endl;
    std::cout << imgData.compressedData.size() << std::endl;

    // 解码 JPEG
    decodeJPEG(imgData, imgData.compressedData);

    // 输出解码后的信息以检查正确性（示例输出前5个块的亮度值）
    int count = 0;
    for (int id = 0; id < 6; id++)
    {
        std::cout << "Block " << id << " Y values (8x8):" << std::endl;
        for (int row = 0; row < 8; row++)
        {
            for (int col=0; col < 8; col++)
            {
                std::cout << imgData.Y[id][row*8 + col] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    for (int block = 0; block < imgData.totalYBlocks; ++block) {
        bool flag = false;
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                // std::cout << imgData.Y_blocks_2D[block][row][col] << " ";
                if (imgData.Y_blocks_2D[block][row][col] > 0)
                {
                    flag = true;
                    break;
                }
            }
            if (flag) break;
        }
        if (flag && count < 5)
        {
            count += 1;
            std::cout << "Block " << block << " Y values (8x8):" << std::endl;
            for (int row = 0; row < 8; ++row) {
                for (int col = 0; col < 8; ++col) {
                    std::cout << imgData.Y_blocks_2D[block][row][col] << " ";
                }
                std::cout << std::endl;  // 每行结束换行
            }
            std::cout << std::endl;  // 每个块结束后再换行
        }
    }


    // 指定 BMP 输出文件路径
    std::string outputFilename = "../output/lena_decoded.bmp";
    
    // 保存解码后的图像为 BMP 文件
    // saveAsImage(outputFilename, imgData);
    saveAsBMP(outputFilename, imgData);
    
    return 0;
}
