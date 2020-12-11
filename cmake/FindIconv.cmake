# - Try to find Iconv
# Once done this will define
#
# ICONV_FOUND - system has Iconv
# ICONV_INCLUDE_DIR - the Iconv include directory
# ICONV_LIBRARIES - Link these to use Iconv
#
if(ICONV_INCLUDE_DIR AND ICONV_LIBRARIES)
  # Already in cache, be silent
  set(ICONV_FIND_QUIETLY TRUE)
endif()
find_path(ICONV_INCLUDE_DIR iconv.h)
find_library(iconv_lib NAMES iconv libiconv libiconv-2 c)
if(ICONV_INCLUDE_DIR AND iconv_lib)
  set(ICONV_FOUND TRUE)
endif()
if(ICONV_FOUND)
  # split iconv into -L and -l linker options, so we can set them for pkg-config
  GET_FILENAME_COMPONENT(iconv_path ${iconv_lib} PATH)
  GET_FILENAME_COMPONENT(iconv_name ${iconv_lib} NAME_WE)
  STRING(REGEX REPLACE "^lib" "" iconv_name ${iconv_name})
  set(ICONV_LIBRARIES "-L${iconv_path} -l${iconv_name}")
  if(NOT ICONV_FIND_QUIETLY)
    MESSAGE(STATUS "Found Iconv: ${ICONV_LIBRARIES}")
  endif()
  if(NOT TARGET Iconv::Iconv)
    add_library(Iconv::Iconv INTERFACE IMPORTED)
  endif()
  set_property(TARGET Iconv::Iconv PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${Iconv_INCLUDE_DIRS}")
  set_property(TARGET Iconv::Iconv PROPERTY INTERFACE_LINK_LIBRARIES "${Iconv_LIBRARIES}")
else()
  if(Iconv_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find Iconv")
  endif()
endif()
MARK_AS_ADVANCED(
  ICONV_INCLUDE_DIR
  ICONV_LIBRARIES
)

