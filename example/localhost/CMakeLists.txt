cmake_minimum_required(VERSION 2.8)
project(textsort C CXX)

##
## Options
##

set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib"
  CACHE PATH "Installation directory for libraries (default: prefix/lib).")

##
## Global Dependencies
##

set(djinni_root ../../)
include(${djinni_root}/support-lib/cmake/DjinniJNI.cmake)
djinni_jni_export_vars(${djinni_root})

##
## Shared Library
##

set(textsort_include_dirs ../generated-src/jni/ ../generated-src/cpp/ ../handwritten-src/cpp/)

file(
  GLOB_RECURSE textsort_srcs
  ../generated-src/jni/*.cpp
  ../generated-src/cpp/*.cpp
  ../handwritten-src/cpp/*.cpp)

set(textsort_common_flags "-Wall -Werror -std=c++1y")

add_library(TextSortNative SHARED ${textsort_srcs} ${DJINNI_JNI_SRCS})
set_target_properties(TextSortNative PROPERTIES COMPILE_FLAGS "${textsort_common_flags} ${DJINNI_JNI_DEFINITIONS}")
include_directories(TextSortNative ${textsort_include_dirs} ${DJINNI_JNI_INCLUDE_DIRS})
target_link_libraries(TextSortNative ${DJINNI_JNI_LIBRARIES})
install(
  TARGETS TextSortNative
  ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
  LIBRARY DESTINATION "${LIB_INSTALL_DIR}")

