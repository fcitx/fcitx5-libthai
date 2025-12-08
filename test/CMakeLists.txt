add_subdirectory(inputmethod)
add_subdirectory(addon)

configure_file(testdir.h.in ${CMAKE_CURRENT_BINARY_DIR}/testdir.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(testlibthai testlibthai.cpp)
target_link_libraries(testlibthai PRIVATE Fcitx5::Core Fcitx5::Module::TestFrontend Fcitx5::Module::TestIM)
add_dependencies(testlibthai libthai copy-addon copy-im)

add_test(testlibthai COMMAND testlibthai)