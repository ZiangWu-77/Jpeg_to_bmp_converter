#include <iostream>
#include <cmath>

const double PI = M_PI;
const int MAT_SIZE = 8;

double DCT_Mat[MAT_SIZE][MAT_SIZE];

// 初始化转换矩阵 A，用于 DCT 和 IDCT
void InitTransMat() {
    for (int i = 0; i < MAT_SIZE; i++) {
        for (int j = 0; j < MAT_SIZE; j++) {
            double a = (i == 0) ? sqrt(1.0 / MAT_SIZE) : sqrt(2.0 / MAT_SIZE);
            DCT_Mat[i][j] = a * cos((j + 0.5) * PI * i / MAT_SIZE);
        }
    }
}

// 执行基于矩阵乘法的正 DCT
void performDCT(const double (&input)[8][8], double (&output)[8][8]) {
    double temp[8][8] = {0};

    // A * input
    for (int i = 0; i < MAT_SIZE; i++) {
        for (int j = 0; j < MAT_SIZE; j++) {
            double sum = 0.0;
            for (int k = 0; k < MAT_SIZE; k++) {
                sum += DCT_Mat[i][k] * input[k][j];
            }
            temp[i][j] = sum;
        }
    }

    // (A * input) * A^T
    for (int i = 0; i < MAT_SIZE; i++) {
        for (int j = 0; j < MAT_SIZE; j++) {
            double sum = 0.0;
            for (int k = 0; k < MAT_SIZE; k++) {
                sum += temp[i][k] * DCT_Mat[j][k];
            }
            output[i][j] = sum;
        }
    }
}

// 执行基于矩阵乘法的逆 DCT
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

// 打印矩阵
void printMatrix(const double (&matrix)[8][8], const std::string &title) {
    std::cout << title << std::endl;
    for (int i = 0; i < MAT_SIZE; i++) {
        for (int j = 0; j < MAT_SIZE; j++) {
            std::cout << matrix[i][j] << "\t";
        }
        std::cout << std::endl;
    }
}

int main() {
    // 初始化 DCT 矩阵
    InitTransMat();

    // 设置一个测试的 8x8 输入矩阵
    double input[8][8] = {
        {84, 23, 40, 72, 70, 67, 5, 80},
        {37, 55, 41, 51, 48, 100, 57, 8},
        {62, 93, 66, 78, 11, 96, 70, 95},
        {73, 34, 84, 49, 66, 6, 96, 92},
        {19, 66, 37, 19, 37, 36, 75, 60},
        {90, 39, 43, 70, 14, 55, 74, 25},
        {57, 63, 59, 98, 57, 26, 43, 87},
        {63, 70, 57, 81, 82, 60, 63, 51}
    };

    double dctOutput[8][8] = {0};
    double idctOutput[8][8] = {0};

    // 执行正 DCT
    performDCT(input, dctOutput);
    printMatrix(dctOutput, "DCT Matrix (Frequency Domain)");

    // 执行逆 DCT
    performInverseDCT(dctOutput, idctOutput);
    printMatrix(idctOutput, "IDCT Matrix (Restored)");

    return 0;
}
