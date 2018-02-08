cmake_minimum_required(VERSION 3.5)
project(uploader)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(src)
#file(GLOB resources "${CMAKE_SOURCE_DIR}/uploader/*")
#foreach(res ${resources})
#    file(COPY ${res} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource)
#endforeach()
