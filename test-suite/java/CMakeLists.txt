cmake_minimum_required(VERSION 3.1)                                                                                                                                                    
project(djinni-test-suite C CXX)


##
## Global Dependencies
##

if(NOT ENV{JAVA_HOME}) 
  message(STATUS "Trying to find Java; to override, set $JAVA_HOME")
  find_package(Java)
  find_package(Java COMPONENTS Runtime)
  find_package(Java COMPONENTS Development)
  if (NOT Java_Development_FOUND)
    message(STATUS "Can't find JDK components; did you only install a JRE?")
  endif()
endif() 

find_package(JNI)

if (NOT JNI_FOUND)
  message(FATAL_ERROR "Could not find JNI.  Did you install a JDK?")
endif()


##
## Test Suite Shared Library
##

set(support_dir ../../support-lib/jni)
set(test_include_dirs ../generated-src/jni/ ../generated-src/cpp/ ../handwritten-src/cpp/)

file(
  GLOB_RECURSE support_srcs
  ${support_dir}/*.cpp)

file(
  GLOB_RECURSE test_suite_srcs
  ../generated-src/jni/*.cpp
  ../generated-src/cpp/*.cpp
  ../handwritten-src/cpp/*.cpp)

set(test_suite_common_flags "-g -Wall -Werror -std=c++1y")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${test_suite_common_flags}")
if(UNIX OR APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

set(test_lib_srcs ${support_srcs} ${test_suite_srcs}) 

add_library(DjinniTestNative SHARED ${test_lib_srcs})
include_directories(
  DjinniTestNative
  ${test_include_dirs}
  ${support_dir}
  ${JNI_INCLUDE_DIRS})
target_link_libraries(DjinniTestNative ${JNI_LIBRARIES})

