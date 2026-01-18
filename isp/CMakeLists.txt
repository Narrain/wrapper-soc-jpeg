# add_library(isp
#     isp.cpp
#     isp.h
# )

# target_include_directories(isp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# target_link_libraries(isp PUBLIC SystemC::systemc common)


cmake_minimum_required(VERSION 3.12)
project(isp_ip LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Find SystemC
find_package(SystemCLanguage REQUIRED)

# -----------------------------
# ISP RTL Library
# -----------------------------
add_library(isp_rtl
    rtl/isp.cpp
    rtl/isp.h
    rtl/pixel.h
)

target_include_directories(isp_rtl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/rtl
)

target_link_libraries(isp_rtl PUBLIC
    SystemC::systemc
)

# -----------------------------
# ISP Testbench Executable
# -----------------------------
add_executable(tb_isp
    tb/sc_main.cpp
    tb/tb_isp.cpp
)

target_include_directories(tb_isp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/rtl
    ${CMAKE_CURRENT_SOURCE_DIR}/tb
)

target_link_libraries(tb_isp PRIVATE
    isp_rtl
    SystemC::systemc
)

# -----------------------------
# Python directory (no build)
# -----------------------------
message(STATUS "Python utilities located in: ${CMAKE_CURRENT_SOURCE_DIR}/python")
