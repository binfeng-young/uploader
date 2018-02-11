
set(qssp_srcs
        common.h
        port.cpp port.h
        qssp.cpp qssp.h
        qsspt.cpp qsspt.h
        )

add_library(qssp ${qssp_srcs} ${moc_srcs})
target_link_libraries(qssp ${QT_LIBRARIES})

add_executable(qssp_test main.cpp)
target_link_libraries(qssp_test qssp)