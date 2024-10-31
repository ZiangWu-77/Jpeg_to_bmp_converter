from struct import unpack

marker_mapping = {
    0xffd8: "Start of Image",
    0xffe0: "Application Default Header",
    0xffdb: "Quantization Table",
    0xffc0: "Start of Frame",
    0xffc4: "Define Huffman Table",
    0xffda: "Start of Scan",
    0xffd9: "End of Image"
}

class JPEG:
    def __init__(self, image_file):
        with open(image_file, 'rb') as f:
            self.img_data = f.read()
    
    def decode(self):
        data = self.img_data
        sos_data = None  # 用于保存 Start of Scan 数据
        while True:
            marker, = unpack(">H", data[0:2])
            print(marker_mapping.get(marker))

            if marker == 0xffd8:
                data = data[2:]
            elif marker == 0xffd9:
                return
            elif marker == 0xffda:
                sos_data = data[2:-2]  # 提取 SOS 段数据，忽略文件末尾的 EOI（End of Image）
                breakpoint()
                with open("../input/sos_segment.bin", "wb") as f:
                    f.write(sos_data)  # 将 SOS 段数据写入 .bin 文件
                return
            else:
                lenchunk, = unpack(">H", data[2:4])
                data = data[2+lenchunk:]            

            if len(data) == 0:
                break

if __name__ == "__main__":
    img = JPEG('../input/lena.jpg')
    img.decode()
