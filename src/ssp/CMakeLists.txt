set(ssp_srcs
        ssp.cpp ssp.h
        sspt.cpp sspt.cpp
        port.cpp port.h
        common.h
        )
add_library(ssp ${ssp_srcs})
if (UNIX)
    set(libs serialport pthread)
else()
    set(libs Qt5::SerialPort)
endif()

target_link_libraries(ssp ${libs})