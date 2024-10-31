cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

project(JPEGtoBMPConverter)
include_directories(include)
add_executable(jpeg_parser
    src/main.cpp
    src/jpeg_header_parser.cpp
    src/jpeg_header_helpers.cpp
    src/huffman_decoder.cpp
    src/inverse_dct.cpp
    src/inverse_quantize.cpp
    src/inverse_zigzag.cpp
    src/jpeg_decoder.cpp
    src/save_as_bmp.cpp
    src/save_as_gray.cpp
)
# 查找 OpenCV 包
find_package(OpenCV REQUIRED)

target_link_libraries(jpeg_parser jpeg ${OpenCV_LIBS})