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
# libjpeg-turbo source + build folders
# ------------------------------------------------------------
set(LIBJPEG_SRC   "${CMAKE_CURRENT_SOURCE_DIR}/../libjpeg-turbo/src")
set(LIBJPEG_BUILD "${CMAKE_CURRENT_SOURCE_DIR}/../libjpeg-turbo/build")

# ------------------------------------------------------------
# Encoder RTL library
# ------------------------------------------------------------
add_library(encoder_rtl
    rtl/jpeg_encoder.cpp
    rtl/jpeg_encoder.h
)

# SystemC + libjpeg include dirs for encoder
target_include_directories(encoder_rtl PUBLIC
    ${SYSTEMC_HOME}/include
    ${LIBJPEG_SRC}
    ${LIBJPEG_BUILD}
    ${CMAKE_CURRENT_SOURCE_DIR}/rtl
    ${CMAKE_CURRENT_SOURCE_DIR}/../isp/rtl
)

# SystemC + libjpeg link dirs
target_link_directories(encoder_rtl PUBLIC
    ${SYSTEMC_HOME}/lib-linux64
    ${LIBJPEG_BUILD}
)

# Link libraries
target_link_libraries(encoder_rtl
    SystemC::systemc
    jpeg
    turbojpeg
)

# ------------------------------------------------------------
# Testbench executable
# ------------------------------------------------------------
add_executable(tb_encoder
    tb/sc_main.cpp
    tb/tb_encoder.cpp
)

# Testbench include dirs
target_include_directories(tb_encoder PUBLIC
    ${SYSTEMC_HOME}/include
    ${LIBJPEG_SRC}
    ${LIBJPEG_BUILD}
    ${CMAKE_CURRENT_SOURCE_DIR}/rtl
    ${CMAKE_CURRENT_SOURCE_DIR}/../isp/rtl
)

# Testbench link dirs
target_link_directories(tb_encoder PUBLIC
    ${SYSTEMC_HOME}/lib-linux64
    ${LIBJPEG_BUILD}
)

# Link testbench
target_link_libraries(tb_encoder
    encoder_rtl
    SystemC::systemc
    jpeg
    turbojpeg
)
