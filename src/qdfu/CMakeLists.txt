set(dfu_srcs
        qdfu.cpp
        )

set(moc_files qdfu.h)

#QT5_WRAP_CPP(moc_srcs ${moc_files})
#add_library(qdfu ${dfu_srcs} ${moc_srcs})

#target_link_libraries(qdfu ssp dfubase ${QT_LIBRARIES})

#add_executable(dfu_cmd main.cpp)

#target_link_libraries(dfu_cmd qdfu)