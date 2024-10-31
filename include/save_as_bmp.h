#ifndef SAVE_AS_BMP_H
#define SAVE_AS_BMP_H

#include "jpeg_header_parser.h"
#include <string>

// 将解码后的 ImageData 保存为 BMP 文件
bool saveAsBMP(const std::string &filename, const ImageData &imgData);

#endif // SAVE_AS_BMP_H
