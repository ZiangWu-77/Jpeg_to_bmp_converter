#ifndef HUFFMAN_DECODER_H
#define HUFFMAN_DECODER_H

#include <vector>
#include <cstdint>
#include "jpeg_header_parser.h"  // 确保包含了 ImageData 结构定义

// 解码函数
int getHuffmanSymbol(BitStreamReader &reader, const HuffmanTable &table);
int decodeHuffmanDC(BitStreamReader &reader, const HuffmanTable &dcTable);
void decodeHuffmanAC(BitStreamReader &reader, const HuffmanTable &acTable, int *block);
void huffmanDecode(const std::vector<uint8_t> &compressedData, ImageData &imgData);

#endif // HUFFMAN_DECODER_H
