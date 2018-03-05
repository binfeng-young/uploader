set(dfu_srcs
        dfu.cpp
        )

set(moc_files dfu.h)

QT5_WRAP_CPP(moc_srcs ${moc_files})
add_library(dfu ${dfu_srcs} ${moc_srcs})

target_link_libraries(dfu qssp usb)

add_executable(dfu_cmd main.cpp)

target_link_libraries(dfu_cmd dfu)