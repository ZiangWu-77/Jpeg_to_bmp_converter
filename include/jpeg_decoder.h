#ifndef JPEG_DECODER_H
#define JPEG_DECODER_H

#include <vector>
#include <cstdint>
#include "jpeg_header_parser.h"

void decodeJPEG(ImageData &imgData, const std::vector<uint8_t> &compressedData);

#endif // JPEG_DECODER_H
