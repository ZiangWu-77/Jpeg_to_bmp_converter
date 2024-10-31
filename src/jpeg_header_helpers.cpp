#include "jpeg_parser_helpers.h"

// 在实现文件中定义函数
uint16_t readBigEndian16(std::ifstream &file) {
    uint8_t highByte = file.get();
    uint8_t lowByte = file.get();
    return (highByte << 8) | lowByte;
}
