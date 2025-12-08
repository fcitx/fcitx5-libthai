add_custom_target(copy-im DEPENDS libthai.conf.in-fmt)
add_custom_command(TARGET copy-im POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/src/libthai.conf ${CMAKE_CURRENT_BINARY_DIR}/libthai.conf)
