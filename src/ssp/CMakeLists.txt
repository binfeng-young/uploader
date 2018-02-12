set(ssp_srcs
        ssp.cpp ssp.h
        sspt.cpp sspt.cpp
        port.cpp port.h
        common.h
        #fifo_buffer.cpp fifo_buffer.h
        )
include_directories(../serialport)
add_library(ssp ${ssp_srcs})
target_link_libraries(ssp serialport pthread)