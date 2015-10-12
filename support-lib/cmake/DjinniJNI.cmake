#
# A macro to help build Djinni JNI support into your library.
#
#   The Djinni support library includes a small amount of non-header
#   code as well as (sometimes optional) symbols that a compiled
#   library *must* export for JNI interoperability. Thus Djinni
#   functions better as a compiled-in 'plugin' to your own
#   library than as a standalone (statically- or dynamically-
#   linked) library.
#
# Exported variables:
#   DJINNI_JNI_INCLUDES
#     Include these header files.
#   DJINNI_JNI_SRCS
#     Djinni source files to compile into your library.
#   DJINNI_JNI_LIBRARIES
#     Link against these (System JNI) libraries.
#   DJINNI_JNI_DEFINITIONS
#     Compiler flags required for building with Djinni.
#
# Example usage:
#   set(djinni_root path/to/djinni/root)
#   include(${djinni_root}/support-lib/cmake/DjinniJNI.cmake)
#   djinni_jni_export_vars(${djinni_root})
#     ...
#   add_library(MyLib SHARED ${my_srcs} ${DJINNI_JNI_SRCS})
#   include_directories(MyLib ${my_includes} ${DJINNI_JNI_INCLUDE_DIRS})
#   set_target_properties(MyLib PROPERTIES COMPILE_FLAGS ${DJINNI_JNI_DEFINITIONS})
#   target_link_libraries(MyLib ${my_deps} ${DJINNI_JNI_LIBRARIES})
#
#  Users of MyLib should use djinni_jni_export_vars("path/to/djinni/root") as well
#

function(DJINNI_JNI_EXPORT_VARS djinni_root)

  if(NOT EXISTS "${djinni_root}" OR NOT IS_DIRECTORY "${djinni_root}")
    message(SEND_ERROR, "Provided DJINNI_ROOT does not exist or is not a directory " ${djinni_root})
	return()
  endif()

  # Djinni requires JNI
  find_package(JNI)
  if(NOT JNI_FOUND)
    message(
      SEND_ERROR
      "Could not find JNI. Did you install a JDK? Set $JAVA_HOME to override")
	return()
  endif()

  set(DJINNI_JNI_INCLUDE_DIRS
    ${djinni_root}/support-lib
	${djinni_root}/support-lib/jni
	${JNI_INCLUDE_DIRS}
	PARENT_SCOPE)
  file(
    GLOB_RECURSE DJINNI_JNI_INCLUDES
    ${djinni_root}/support-lib/*.hpp
    ${JNI_INCLUDE_DIRS})
  file(
    GLOB_RECURSE DJINNI_JNI_SRCS
    ${djinni_root}/support-lib/*.cpp)
  set(DJINNI_JNI_SRCS ${DJINNI_JNI_SRCS} PARENT_SCOPE)
  
  set(DJINNI_JNI_LIBRARIES ${JNI_LIBRARIES} PARENT_SCOPE)
  set(DJINNI_JNI_DEFINITIONS -std=c++1y PARENT_SCOPE)
  if(UNIX OR APPLE)
    set(DJINNI_JNI_DEFINITIONS "${DJINNI_JNI_DEFINITIONS} -fPIC" PARENT_SCOPE)
  endif()

endfunction()

