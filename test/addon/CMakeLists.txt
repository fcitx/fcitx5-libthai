cmake_policy(SET CMP0175 NEW)

add_custom_target(copy-addon DEPENDS
    libthai-addon.conf.in-fmt
    )
add_custom_command(TARGET copy-addon POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/src/libthai-addon.conf ${CMAKE_CURRENT_BINARY_DIR}/libthai.conf)
