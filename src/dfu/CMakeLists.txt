set(dfu_src
        dfu.cpp
        )

set(moc_files dfu.h)
set(uic_files mainwindow.ui)

QT5_WRAP_CPP(moc_srcs ${moc_files})
add_library(dfu ${dfu_src} ${moc_srcs})

target_link_libraries(dfu ssp)

add_executable(dfu_cmd main.cpp)

target_link_libraries(dfu_cmd dfu)