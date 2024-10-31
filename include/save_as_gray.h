#ifndef SAVE_AS_GRAY_H
#define SAVE_AS_GRAY_H

#include "jpeg_header_parser.h"
#include <string>

// 将解码后的 ImageData 保存为 BMP 文件
bool saveAsImage(const std::string &filename, const ImageData &imgData);

#endif // SAVE_AS_BMP_H