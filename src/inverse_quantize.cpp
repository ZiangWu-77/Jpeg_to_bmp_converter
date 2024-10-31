#include "inverse_quantize.h"

// 逆量化：应用量化表
void inverseQuantize(ImageData &imgData) 
{
    // 获取量化表
    const std::vector<int> &quantTableY = imgData.quantizationTables[imgData.yQuantTableId];
    const std::vector<int> &quantTableCrCb = imgData.quantizationTables[imgData.crCbQuantTableId];


    // 对每个 MCU 的 4 个 Y 分量块应用量化表 0
    for (int mcu = 0; mcu < imgData.totalBlocks; ++mcu) 
    {
        for (int yBlock = 0; yBlock < 4; ++yBlock) 
        {
            int blockIndex = mcu * 4 + yBlock; // 每个 MCU 中的第 yBlock 个 Y 块
            for (int i = 0; i < 64; i ++ )
            {
                imgData.Y[blockIndex][i] *= quantTableY[i];
            }
        }

        // 对每个 Cr 和 Cb 分量块应用量化表 1
        // 对 8x8 的 Cr 块进行逐元素逆量化
        for (int i = 0; i < 64; i ++ )
        {
            imgData.Cr[mcu][i] *= quantTableCrCb[i];
            imgData.Cb[mcu][i] *= quantTableCrCb[i];
        }
    }
}
