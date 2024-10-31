#include "jpeg_header_parser.h"

void inverseZigZag(ImageData &imgData)
{
    // Zig-Zag 索引表
    const int zigzagOrder[8][8] = 
    {{0,  1,  5,  6, 14, 15, 27, 28},
    {2,  4,  7, 13, 16, 26, 29, 42},
    {3,  8, 12, 17, 25, 30, 41, 43},
    {9, 11, 18, 24, 31, 40, 44, 53},
    {10, 19, 23, 32, 39, 45, 52, 54},
    {20, 22, 33, 38, 46, 51, 55, 60},
    {21, 34, 37, 47, 50, 56, 59, 61},
    {35, 36, 48, 49, 57, 58, 62, 63}};

    for (int mcu = 0; mcu < imgData.totalBlocks; mcu++)
    {
        for (int yBlock = 0; yBlock < 4; yBlock++)
        {
            int blockIndex = mcu * 4 + yBlock;
            for (int row = 0; row < 8; row++)
            {
                for (int col = 0; col < 8; col++)
                {
                    int Y = imgData.Y[blockIndex][zigzagOrder[row][col]];
                    imgData.Y_blocks_2D[blockIndex][row][col] = Y;
                }
            }
        }

        for (int row = 0; row < 8; row++)
        {
            for (int col = 0; col < 8; col++)
            {
                imgData.Cr_blocks_2D[mcu][row][col] = imgData.Cr[mcu][zigzagOrder[row][col]];
            }
        }

        for (int row = 0; row < 8; row++)
        {
            for (int col = 0; col < 8; col++)
            {
                imgData.Cb_blocks_2D[mcu][row][col] = imgData.Cb[mcu][zigzagOrder[row][col]];
            }
        }
    }
}