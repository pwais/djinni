##
## CMake settings for DjinniX JNI-Arrays
##
##   Set $DJINNI_ROOT to the root of your djinni repo
##   and include this file to build JNI-Arrays into
##   your library.  This module exports the following
##   variables:
##     $DJINNIX_JNI_ARRAYS_HEADERS
##     $DJINNIX_JNI_ARRAYS_SOURCES
##     $JNI_INCLUDE_DIRS
##     
##


cmake_minimum_required(VERSION 2.8)
project(djinnix-jni-arrays C CXX)

##
## Options
##

set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib"
  CACHE PATH "Installation directory for libraries (default: prefix/lib).")
set(INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include"
  CACHE PATH
  "Installation directory for header files (default: prefix/include).")

##
## Global Dependencies
##

find_package(JNI)
if (NOT JNI_FOUND)
  message(
    FATAL_ERROR
	"Could not find JNI. Did you install a JDK? Set $JAVA_HOME to override")
endif()


##
## JNI-Arrays Shared Library
##

set(support_dir ../../support-lib/jni)
set(
  arrays_include_dirs
  build/generated-src/jni/ 
  build/generated-src/cpp/
  src/cpp/)

file(
  GLOB_RECURSE support_srcs
  ${support_dir}/*.cpp)

file(
  GLOB_RECURSE arrays_srcs
  build/generated-src/jni/*.cpp
  build/generated-src/cpp/*.cpp
  src/cpp/*.cpp)

set(arrays_common_flags "-g -Wall -Werror -std=c++1y")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${arrays_common_flags}")
if(UNIX OR APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

set(arrays_lib_srcs ${support_srcs} ${arrays_srcs}) 

add_library(DjinniXJNIArrays SHARED ${arrays_lib_srcs})
include_directories(
  DjinniXJNIArrays
  ${arrays_include_dirs}
  ${support_dir}
  ${JNI_INCLUDE_DIRS})
target_link_libraries(DjinniXJNIArrays ${JNI_LIBRARIES})
install(
  TARGETS DjinniXJNIArrays
  ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
  LIBRARY DESTINATION "${LIB_INSTALL_DIR}")
install(
  DIRECTORY ${arrays_include_dirs}
  DESTINATION "${INSTALL_INCLUDE_DIR}"
  FILES_MATCHING PATTERN "*.hpp")

#add_subdirectory(test)

##
## Test Shared Library
##

#set(test_include_dirs generated-src/jni/ generated-src/cpp/ src/cpp/ test/cpp/)
#
#file(
#  GLOB_RECURSE test_srcs
#  test/cpp/*.cpp
#  test/generated-src/jni/*.cpp
#  test/generated-src/cpp/*.cpp)
#
#set(test_lib_srcs ${support_srcs} ${arrays_srcs} ${test_srcs})
#
#add_library(DjinniXJNIArraysTest SHARED ${test_lib_srcs})
#include_directories(
#  DjinniXJNIArraysTest
#  ${arrays_include_dirs}
#  ${test_include_dirs}
#  ${support_dir}
#  ${JNI_INCLUDE_DIRS})
#target_link_libraries(DjinniXJNIArraysTest DjinniXJNIArrays ${JNI_LIBRARIES})
#install(
#  TARGETS DjinniXJNIArraysTest
#  ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
#  LIBRARY DESTINATION "${LIB_INSTALL_DIR}")

