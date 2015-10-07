cmake_minimum_required(VERSION 2.8)
project(djinni-test-suite C CXX)

##
## Options
##

set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib"
  CACHE PATH "Installation directory for libraries (default: prefix/lib).")

##
## Global Dependencies
##

set(DJINNI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../../support-lib/cmake")
find_package(DjinniJNI REQUIRED)
if(NOT DJINNI_JNI_FOUND)
  message(
    FATAL_ERROR
	"Could not find Djinni JNI Support.  Did you run `make jni_support` in the root of the repo?")
endif()

##
## Test Suite Shared Library
##

#set(support_dir ../../support-lib/jni)
set(test_include_dirs ../generated-src/jni/ ../generated-src/cpp/ ../handwritten-src/cpp/)

#file(
#  GLOB_RECURSE support_srcs
#  ${support_dir}/*.cpp)

file(
  GLOB_RECURSE test_suite_srcs
  ../generated-src/jni/*.cpp
  ../generated-src/cpp/*.cpp
  ../handwritten-src/cpp/*.cpp)

set(test_suite_common_flags "-g -Wall -Werror -std=c++1y")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${test_suite_common_flags} ${DJINNI_JNI_DEFINITIONS}")
if(UNIX OR APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

#set(test_lib_srcs ${test_suite_srcs})

add_library(DjinniTestNative SHARED ${test_suite_srcs})
include_directories(
  DjinniTestNative
  ${test_include_dirs}
  ${DJINNI_JNI_INCLUDE_DIRS})
# WOOF! need to export some link flags ... https://github.com/janelia-flyem/buildem/blob/master/cplex-shared.cmake#L7
target_link_libraries(DjinniTestNative "-Wl,-all_load" ${DJINNI_JNI_LIBRARIES})
install(
  TARGETS DjinniTestNative
  ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
  LIBRARY DESTINATION "${LIB_INSTALL_DIR}")

