aux_source_directory(. FILE_LIB_SRCS)
add_library(file_process ${FILE_LIB_SRCS})
target_link_libraries(file_process log)
target_link_libraries(file_process str)
