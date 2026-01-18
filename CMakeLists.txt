cmake_minimum_required(VERSION 3.15)
project(vision_soc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SystemC
set(SYSTEMC_HOME "/opt/systemc-2.3.4")
find_package(SystemCLanguage REQUIRED)

include_directories(
    ${SYSTEMC_HOME}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/encoder/rtl
    ${CMAKE_CURRENT_SOURCE_DIR}/isp/rtl
    ${CMAKE_CURRENT_SOURCE_DIR}/soc_top
    ${CMAKE_CURRENT_SOURCE_DIR}/libjpeg-turbo
    ${CMAKE_CURRENT_SOURCE_DIR}/libjpeg-turbo/src
    ${CMAKE_CURRENT_SOURCE_DIR}/libjpeg-turbo/build
)


link_directories(
    ${SYSTEMC_HOME}/lib-linux64
    ${CMAKE_CURRENT_SOURCE_DIR}/libjpeg-turbo/build
)

# Encoder RTL
add_library(encoder_rtl
    encoder/rtl/jpeg_encoder.cpp
)

target_link_libraries(encoder_rtl
    SystemC::systemc
    jpeg
    turbojpeg
)

# ISP stub
add_library(isp_stub
    isp/rtl/isp_stub.cpp
)

target_link_libraries(isp_stub
    SystemC::systemc
)

# Bitstream sink
add_library(bitstream_sink
    soc_top/BitstreamSink.cpp
)

target_link_libraries(bitstream_sink
    SystemC::systemc
)

# VisionSoC
add_library(vision_soc_lib
    soc_top/VisionSoC.cpp
)

target_link_libraries(vision_soc_lib
    encoder_rtl
    isp_stub
    bitstream_sink
    SystemC::systemc
)

# Testbench
add_executable(vision_soc_tb
    tb/sc_main.cpp
)

target_link_libraries(vision_soc_tb
    vision_soc_lib
    SystemC::systemc
)
