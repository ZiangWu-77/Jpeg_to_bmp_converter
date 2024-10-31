#ifndef JPEG_PARSER_HELPERS_H
#define JPEG_PARSER_HELPERS_H

#include <fstream>
#include <cstdint>

// 仅在头文件中声明
uint16_t readBigEndian16(std::ifstream &file);

#endif // JPEG_PARSER_HELPERS_H
