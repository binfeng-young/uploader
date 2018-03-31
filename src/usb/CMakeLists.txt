add_definitions(-DOPHID_LIBRARY)

set(usb_srcs
        ophid_usbsignal.cpp
        ophid.cpp
        ophid_hidapi.cpp
        hidapi.h
        ophid_const.h
        ophid_usbmon.h
        )

set(moc_files ophid.h ophid_hidapi.h ophid_usbmon.h ophid_usbsignal.h)

QT5_WRAP_CPP(moc_srcs ${moc_files})

if (WIN32)
    set(usb_srcs ${usb_srcs} windows/ophid_usbmon.cpp
            windows/hid.c
            )
    set(usb_libs hid setupapi)
ELSEIF (APPLE)
    set(usb_srcs ${usb_srcs} mac/ophid_usbmon.cpp
            mac/hid.c)
    find_library(CoreFoundation_lib CoreFoundation)
    find_library(IOKit_lib IOKit)
    set(usb_libs ${CoreFoundation_lib} ${IOKit_lib})
ELSEIF (UNIX)
    set(usb_srcs ${usb_srcs} linux/ophid_usbmon.cpp
            linux/hid.c
            linux/libusbhid.c)
    set(usb_libs udev rt pthread)
endif ()

add_library(usb ${usb_srcs} ${moc_srcs})
target_link_libraries(usb ${usb_libs} ${QT_LIBRARIES})
