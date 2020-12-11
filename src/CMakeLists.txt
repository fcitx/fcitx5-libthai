
add_library(iconvwrapper STATIC iconvwrapper.cpp)
target_link_libraries(iconvwrapper Fcitx5::Utils Iconv::Iconv)

set(LIBTHAI_SOURCES
    engine.cpp
    thaikb.cpp
)
add_library(libthai MODULE ${LIBTHAI_SOURCES})
target_link_libraries(libthai iconvwrapper Fcitx5::Core PkgConfig::LibThai Iconv::Iconv)
target_include_directories(libthai PRIVATE ${PROJECT_BINARY_DIR})
set_target_properties(libthai PROPERTIES PREFIX "")
install(TARGETS libthai DESTINATION "${CMAKE_INSTALL_LIBDIR}/fcitx5")
fcitx5_translate_desktop_file(libthai.conf.in libthai.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libthai.conf" DESTINATION "${CMAKE_INSTALL_DATADIR}/fcitx5/inputmethod")
fcitx5_translate_desktop_file(libthai-addon.conf.in libthai-addon.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libthai-addon.conf" RENAME libthai.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")

