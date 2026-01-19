cmake_minimum_required(VERSION 3.15)
project(vision_soc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-g)

# ------------------------------------------------------------
# SystemC
# ------------------------------------------------------------
set(SYSTEMC_HOME "/opt/systemc-2.3.4")
find_package(SystemCLanguage REQUIRED)

# ------------------------------------------------------------
# Global include paths
# ------------------------------------------------------------
include_directories(
    ${SYSTEMC_HOME}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/encoder
    ${CMAKE_CURRENT_SOURCE_DIR}/encoder/rtl
    ${CMAKE_CURRENT_SOURCE_DIR}/block
    ${CMAKE_CURRENT_SOURCE_DIR}/isp/rtl
    ${CMAKE_CURRENT_SOURCE_DIR}/soc_top
)

link_directories(
    ${SYSTEMC_HOME}/lib-linux64
)

# ------------------------------------------------------------
# Encoder RTL
# ------------------------------------------------------------
add_library(encoder_rtl
    encoder/rtl/JPEG_Encoder.cpp
    encoder/rtl/JPEG_Encoder.h
    encoder/rtl/DCTQ.cpp
    encoder/rtl/DCTQ.h
    encoder/rtl/Entropy.cpp
    encoder/rtl/Entropy.h
    encoder/rtl/HeaderGen.cpp
    encoder/rtl/HeaderGen.h
)

target_link_libraries(encoder_rtl
    SystemC::systemc
)

# ------------------------------------------------------------
# ISP stub
# ------------------------------------------------------------
add_library(isp_stub
    isp/rtl/isp_stub.cpp
)
target_link_libraries(isp_stub SystemC::systemc)

# ------------------------------------------------------------
# Pixel FIFO
# ------------------------------------------------------------
add_library(pixelfifo
    soc_top/PixelFifo.cpp
)
target_link_libraries(pixelfifo SystemC::systemc)

# ------------------------------------------------------------
# Bitstream FIFO
# ------------------------------------------------------------
add_library(bitstream_fifo
    soc_top/BitstreamFifo.cpp
)
target_link_libraries(bitstream_fifo SystemC::systemc)

# ------------------------------------------------------------
# Block Maker
# ------------------------------------------------------------
add_library(block_maker
    block/BlockMaker.cpp
)
target_link_libraries(block_maker SystemC::systemc)

# ------------------------------------------------------------
# DMA Stub
# ------------------------------------------------------------
add_library(dma_stub
    soc_top/Dmastub.cpp
)
target_link_libraries(dma_stub SystemC::systemc)

# ------------------------------------------------------------
# Bitstream sink
# ------------------------------------------------------------
add_library(bitstream_sink
    soc_top/BitstreamSink.cpp
)
target_link_libraries(bitstream_sink SystemC::systemc)

# ------------------------------------------------------------
# ConfigRegs
# ------------------------------------------------------------
add_library(config_regs
    soc_top/ConfigRegs.cpp
)
target_link_libraries(config_regs SystemC::systemc)

# ------------------------------------------------------------
# FINAL EXECUTABLE (contains sc_main)
# ------------------------------------------------------------
add_executable(vision_soc_tb
    soc_top/VisionSoC.cpp
)

target_link_libraries(vision_soc_tb
    encoder_rtl
    isp_stub
    pixelfifo
    block_maker
    bitstream_fifo
    dma_stub
    config_regs
    SystemC::systemc
)
