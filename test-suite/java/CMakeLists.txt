cmake_minimum_required(VERSION 2.8)
project(djinni-test-suite C CXX)

##
## Options
##

set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib"
  CACHE PATH "Installation directory for libraries (default: prefix/lib).")

##
## Dependencies
##

set(djinni_root ../../)
include(${djinni_root}/support-lib/cmake/DjinniJNI.cmake)
djinni_jni_export_vars(${djinni_root})

##
## Test Suite Shared Library
##

set(test_include_dirs ../generated-src/jni/ ../generated-src/cpp/ ../handwritten-src/cpp/)

file(
  GLOB_RECURSE test_suite_srcs
  ../generated-src/jni/*.cpp
  ../generated-src/cpp/*.cpp
  ../handwritten-src/cpp/*.cpp)

set(test_suite_common_flags "-g -Wall -Werror")

add_library(DjinniTestNative SHARED ${test_suite_srcs} ${DJINNI_JNI_SRCS})
set_target_properties(DjinniTestNative PROPERTIES COMPILE_FLAGS "${test_suite_common_flags} ${DJINNI_JNI_DEFINITIONS}")
include_directories(DjinniTestNative ${test_include_dirs} ${DJINNI_JNI_INCLUDE_DIRS})
target_link_libraries(DjinniTestNative ${DJINNI_JNI_LIBRARIES})
install(
  TARGETS DjinniTestNative
  ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
  LIBRARY DESTINATION "${LIB_INSTALL_DIR}")

