#include "inverse_dct.h"
#include <cmath>
#include <vector>

const double PI = M_PI;
const int MAT_SIZE = 8; // 固定为8x8 DCT块

double DCT_Mat[MAT_SIZE][MAT_SIZE];
float DctMapTmp[MAT_SIZE][MAT_SIZE];

// 初始化转换矩阵 A，用于 DCT 和 IDCT
void InitTransMat()
{
    int i, j;
    float a;

    for (i = 0; i < MAT_SIZE; i++)
    {
        for (j = 0; j < MAT_SIZE; j++)
        {
            if (i == 0)
                a = sqrt(1.0 / MAT_SIZE);
            else
                a = sqrt(2.0 / MAT_SIZE);
            DCT_Mat[i][j] = a * cos((j + 0.5) * PI * i / MAT_SIZE);
        }
    }
}

// 执行基于矩阵乘法的逆 DCT，修改为引用传递
void performInverseDCT(const double (&input)[8][8], double (&output)[8][8]) {
    double temp[8][8] = {0};

    // A^T * input
    for (int i = 0; i < MAT_SIZE; i++) {
        for (int j = 0; j < MAT_SIZE; j++) {
            double sum = 0.0;
            for (int k = 0; k < MAT_SIZE; k++) {
                sum += DCT_Mat[k][i] * input[k][j];
            }
            temp[i][j] = sum;
        }
    }

    // (A^T * input) * A
    for (int i = 0; i < MAT_SIZE; i++) {
        for (int j = 0; j < MAT_SIZE; j++) {
            double sum = 0.0;
            for (int k = 0; k < MAT_SIZE; k++) {
                sum += temp[i][k] * DCT_Mat[k][j];
            }
            output[i][j] = sum;
        }
    }
}

// 对 ImageData 中的 Y、Cr、Cb 数据块执行逆 DCT
void inverseDCT(ImageData &imgData) {
    InitTransMat(); // 初始化 DCT 矩阵

    for (int mcu = 0; mcu < imgData.totalBlocks; ++mcu) {
        for (int yBlock = 0; yBlock < 4; ++yBlock) {
            int blockIndex = mcu * 4 + yBlock;
            double temp[8][8], result[8][8];

            // 将 1D Y 数据转换为 2D
            for (int row = 0; row < 8; ++row) {
                for (int col = 0; col < 8; ++col) {
                    temp[row][col] = imgData.Y_blocks_2D[blockIndex][row][col];
                }
            }

            // 执行逆 DCT
            performInverseDCT(temp, result);

            // 将结果写回 Y_blocks_2D
            for (int row = 0; row < 8; ++row) {
                for (int col = 0; col < 8; ++col) {
                    imgData.Y_blocks_2D[blockIndex][row][col] = static_cast<int>(std::round(result[row][col]));
                }
            }
        }

        // 对 Cr 和 Cb 块执行逆 DCT
        double tempCr[8][8], resultCr[8][8];
        double tempCb[8][8], resultCb[8][8];

        // Cr 块
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                tempCr[row][col] = imgData.Cr_blocks_2D[mcu][row][col];
            }
        }
        performInverseDCT(tempCr, resultCr);
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                imgData.Cr_blocks_2D[mcu][row][col] = static_cast<int>(std::round(resultCr[row][col]));
            }
        }

        // Cb 块
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                tempCb[row][col] = imgData.Cb_blocks_2D[mcu][row][col];
            }
        }
        performInverseDCT(tempCb, resultCb);
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                imgData.Cb_blocks_2D[mcu][row][col] = static_cast<int>(std::round(resultCb[row][col]));
            }
        }
    }
}
