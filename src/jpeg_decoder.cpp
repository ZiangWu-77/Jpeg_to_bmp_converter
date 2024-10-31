#include "jpeg_decoder.h"
#include "huffman_decoder.h"
#include "jpeg_parser_helpers.h"
#include "inverse_dct.h"   // 假设逆DCT放在此文件中
#include "inverse_quantize.h" // 假设逆量化放在此文件中
#include "inverse_zigzag.h"

void decodeJPEG(ImageData &imgData, const std::vector<uint8_t> &compressedData) {
    // Step 1: 哈夫曼解码
    huffmanDecode(compressedData, imgData);
    // Step 2: 逆量化
    inverseQuantize(imgData); // 使用量化表
    // step 3: zigzag
    inverseZigZag(imgData);
    // Step 4: 逆 DCT
    inverseDCT(imgData);
}