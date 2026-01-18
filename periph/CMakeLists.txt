add_library(periph
    i2c.cpp
    i2c.h
)

target_include_directories(periph PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(periph PUBLIC SystemC::systemc common)
