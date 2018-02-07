set(exec_name ${PROJECT_NAME})

set (CMAKE_PREFIX_PATH "C:/Qt/Qt5.10.0/5.10.0/mingw53_32")
find_package(Qt5 REQUIRED COMPONENTS Widgets SerialPort)
set(QT_LIBRARIES Qt5::Widgets Qt5::SerialPort)

include_directories(ssp dfu)

add_subdirectory(ssp)
add_subdirectory(dfu)
set(baseic_srcs
        main.cpp
        mainwindow.cpp
        serialdevice.cpp
        )
set(moc_files mainwindow.h serialdevice.h)
set(uic_files mainwindow.ui)

QT5_WRAP_CPP(moc_srcs ${moc_files})
QT5_WRAP_UI(uic_srcs ${uic_files})

set(srcs
        ${baseic_srcs}
        ${moc_srcs}
        ${uic_srcs})
add_executable(${exec_name} ${srcs})
#set_target_properties(CameraCalibrationTool PROPERTIES ENVIRONMENT "${PROJECT_DEPENDS_PATH}")
target_link_libraries(${exec_name}  ${QT_LIBRARIES})