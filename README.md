## Intro
This is a wheel project aiming to decode jpeg and transfer it to bmp. Proj structure is below.
```
.
├── CMakeLists.txt
├── include
│   ├── huffman_decoder.h
│   ├── inverse_dct.h
│   ├── inverse_quantize.h
│   ├── inverse_zigzag.h
│   ├── jpeg_decoder.h
│   ├── jpeg_header_parser.h
│   ├── jpeg_parser_helpers.h
│   ├── save_as_bmp.h
│   └── save_as_gray.h
├── input
│   ├── lena.jpg
│   ├── sos_compressed_data.bin
│   └── sos_segment.bin
├── output
│   └── lena_decoded.bmp
├── src
│   ├── huffman_decoder.cpp
│   ├── inverse_dct.cpp
│   ├── inverse_quantize.cpp
│   ├── inverse_zigzag.cpp
│   ├── jpeg_decoder.cpp
│   ├── jpeg_decompress.py
│   ├── jpeg_header_helpers.cpp
│   ├── jpeg_header_parser.cpp
│   ├── main.cpp
│   ├── save_as_bmp.cpp
│   └── save_as_gray.cpp
└── structure.txt

4 directories, 26 files
```
## Build
```
mkdir build
cd build
cmake ..
make
```
## Run
```
./jpeg_parser
```
## Result
You can see the *.bmp in output folder(default be the lena photo)
