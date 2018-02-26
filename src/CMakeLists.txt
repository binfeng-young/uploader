set(exec_name ${PROJECT_NAME})

set (CMAKE_PREFIX_PATH "C:/cygwin64")
find_package(Qt5 REQUIRED COMPONENTS Widgets SerialPort)
set(QT_LIBRARIES Qt5::Widgets Qt5::SerialPort)

include_directories(serialport ssp qssp dfu qdfu)
add_subdirectory(serialport)
add_subdirectory(ssp)
#add_subdirectory(qssp)
add_subdirectory(dfu)
add_subdirectory(qdfu)
set(baseic_srcs
        main.cpp
        devicewidget.cpp
        uploaderwidget.cpp
        #rebootdialog.cpp
        enums.h
        devicedescriptorstruct.h
        utils.h
        )
set(moc_files
        uploaderwidget.h
        devicewidget.h
        #rebootdialog.h
        )
set(uic_files
        uploader.ui
        devicewidget.ui
        #rebootdialog.ui
        )
set(qrc_files uploader.qrc)

QT5_WRAP_CPP(moc_srcs ${moc_files})
QT5_WRAP_UI(uic_srcs ${uic_files})
QT5_ADD_RESOURCES(qrc_srcs ${qrc_files})

set(srcs
        ${baseic_srcs}
        ${moc_srcs}
        ${uic_srcs}
        ${qrc_srcs})
add_executable(${exec_name} ${srcs})
#set_target_properties(CameraCalibrationTool PROPERTIES ENVIRONMENT "${PROJECT_DEPENDS_PATH}")
target_link_libraries(${exec_name}  ${QT_LIBRARIES} qdfu)