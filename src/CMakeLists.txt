set(exec_name ${PROJECT_NAME})

set(baseic_srcs main.cpp)
set(srcs ${baseic_srcs})
add_executable(${exec_name} ${srcs})
#target_link_libraries(${exec_name} ${})