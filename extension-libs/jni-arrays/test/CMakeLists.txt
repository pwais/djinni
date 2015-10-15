cmake_minimum_required(VERSION 2.8)
project(djinnix-jni-arrays-test C CXX)

##
## Options
##

set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib"
  CACHE PATH "Installation directory for libraries (default: prefix/lib).")

##
## Dependencies
##

set(djinni_root ../../../)
include(${djinni_root}/support-lib/cmake/DjinniJNI.cmake)
djinni_jni_export_vars(${djinni_root})

include(${djinni_root}/extension-libs/jni-arrays/cmake/DjinniXJNIArrays.cmake)
djinni_jni_arrays_export_vars()

##
## Test Shared Library
##

set(
  test_include_dirs
  build/generated-src/jni/
  build/generated-src/cpp/
  src/cpp/)

file(
  GLOB_RECURSE test_srcs
  src/cpp/*.cpp
  build/generated-src/jni/*.cpp
  build/generated-src/cpp/*.cpp)

set(test_flags "-g3 -Wall -Werror")

add_library(DjinniXJNIArraysTest SHARED ${test_srcs} ${DJINNI_JNI_SRCS})
set_target_properties(DjinniXJNIArraysTest PROPERTIES COMPILE_FLAGS "${test_flags} ${DJINNI_JNI_DEFINITIONS}")
include_directories(DjinniXJNIArraysTest ${test_include_dirs} ${DJINNI_JNI_INCLUDE_DIRS})
target_link_libraries(DjinniXJNIArraysTest ${DJINNI_JNI_LIBRARIES})
install(
  TARGETS DjinniXJNIArraysTest
  ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
  LIBRARY DESTINATION "${LIB_INSTALL_DIR}")

