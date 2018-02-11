set(ssp_srcs
        ssp.cpp ssp.h
        fifo_buffer.cpp fifo_buffer.h
        )
include_directories(../serialport)
add_library(ssp ${ssp_srcs})


add_executable(ssp_test main.cpp)
target_link_libraries(ssp_test ssp serialport)