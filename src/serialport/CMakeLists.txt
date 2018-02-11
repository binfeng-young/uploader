set(sp_srcs BvSerial.c BvSerial.h)

add_library(serialport ${sp_srcs})

add_executable(serialport_test main.cpp)
target_link_libraries(serialport_test serialport)