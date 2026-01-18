add_library(memory
    mem_ctrl.cpp
    mem_ctrl.h
)

target_include_directories(memory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(memory PUBLIC SystemC::systemc common)
