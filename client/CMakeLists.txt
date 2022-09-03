aux_source_directory(. clientLib_LIB_SRC)
add_library(clientLib ${clientLib_LIB_SRC})
target_link_libraries(clientLib log net str file_process progressbar statusbar ${CURSES_LIBRARIES})