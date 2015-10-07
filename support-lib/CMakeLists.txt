cmake_minimum_required(VERSION 2.8)
project(DjinniJNISupport C CXX)

##
## Install Options -- helps support build-local install
##

set(EXEC_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}"
  CACHE PATH "Base installation path.")

set(LIB_INSTALL_DIR "${EXEC_INSTALL_PREFIX}/lib"
  CACHE PATH "Installation directory for shared libraries.")

set(INCLUDE_INSTALL_DIR "${EXEC_INSTALL_PREFIX}/include"
  CACHE PATH "Installation directory for header files.")

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
## Core Settings
##

set(support_dir ./jni)
set(support_common_flags "-Wall -Werror -std=c++1y")

##
## Djinni JNI Support Lib
##   NB: Since this lib defines a JNI_OnLoad(), users will want
##   to link to it rather than compile the support lib sources
##   into their own library.
##

if(UNIX OR APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")                                                                                                                               
endif()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${support_common_flags} -g3 -pg")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${support_common_flags} -O3")

file(
  GLOB_RECURSE support_includes
  ${support_dir}/*.hpp)
file(
  GLOB_RECURSE support_srcs
  ${support_dir}/*.cpp)

add_library(DjinniJNISupport STATIC ${support_srcs})
include_directories(
  DjinniJNISupport
  ${support_includes}
  ${JNI_INCLUDE_DIRS})
target_link_libraries(DjinniJNISupport ${JNI_LIBRARIES})
install(
  TARGETS DjinniJNISupport
  ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
  LIBRARY DESTINATION "${LIB_INSTALL_DIR}")
install(
  DIRECTORY ${support_dir}
  DESTINATION "${INCLUDE_INSTALL_DIR}/djinni"
  FILES_MATCHING PATTERN "*.hpp")

##
## Make Djinni JNI Support Lib and Headers
## available via pkg-config
##

if(NOT MSVC) # No pkg-config on Windows
  # See djinni_support.pc.in for variables used
  set(prefix "${CMAKE_INSTALL_PREFIX}")
  set(libdir "${LIB_INSTALL_DIR}")
  set(includedir "${INCLUDE_INSTALL_DIR}")

  configure_file(djinni_jni.pc.in "${CMAKE_CURRENT_BINARY_DIR}/djinni_jni.pc" @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/djinni_jni.pc" DESTINATION "${LIB_INSTALL_DIR}/pkgconfig")

  unset(includedir)
  unset(libdir)
  unset(prefix)  
endif()

