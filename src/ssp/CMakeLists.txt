
set(ssp_srcs
        common.h
        port.cpp port.h
        ssp.cpp ssp.h
        qssp.cpp qssp.h
        qsspt.cpp qsspt.h
        )

add_library(ssp ${ssp_srcs} ${moc_srcs})
target_link_libraries(ssp ${QT_LIBRARIES})

add_executable(ssp_test main.cpp)
target_link_libraries(ssp_test ssp)