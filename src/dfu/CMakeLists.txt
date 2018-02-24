set(dfubase_src dfubase.h dfubase.cpp)

add_library(dfubase ${dfubase_src})
target_link_libraries(dfubase ssp)

add_executable(dfu_shuttle dfushuttle.cpp)
target_link_libraries(dfu_shuttle dfubase)