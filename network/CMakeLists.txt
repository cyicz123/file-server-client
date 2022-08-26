aux_source_directory(. NET_LIB_SRCS)
link_libraries(pthread)
add_library(net ${NET_LIB_SRCS})
target_link_libraries(net log)