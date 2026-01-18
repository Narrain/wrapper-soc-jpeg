add_library(mcu
    mcu.cpp
    mcu.h
)

target_include_directories(mcu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mcu PUBLIC SystemC::systemc common)
