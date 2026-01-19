cmake_minimum_required(VERSION 3.15)
project(jpeg_encoder_sc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------
# SystemC installation
# ------------------------------------------------------------
set(SYSTEMC_HOME "/opt/systemc-2.3.4")
find_package(SystemCLanguage REQUIRED)

# ------------------------------------------------------------
# Encoder RTL library
# ------------------------------------------------------------
add_library(encoder_rtl
    rtl/EncoderTop.cpp
    rtl/EncoderTop.h
    rtl/JPEG_Encoder.cpp
    rtl/JPEG_Encoder.h
    rtl/DCTQ.cpp
    rtl/DCTQ.h
    rtl/Entropy.cpp
    rtl/Entropy.h
    rtl/HeaderGen.cpp
    rtl/HeaderGen.h
)

target_include_directories(encoder_rtl PUBLIC
    ${SYSTEMC_HOME}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_CURRENT_SOURCE_DIR}/rtl
    ${CMAKE_CURRENT_SOURCE_DIR}/../isp/rtl
)


target_link_directories(encoder_rtl PUBLIC
    ${SYSTEMC_HOME}/lib-linux64
    ${LIBJPEG_BUILD}
)

target_link_libraries(encoder_rtl
    SystemC::systemc
)

