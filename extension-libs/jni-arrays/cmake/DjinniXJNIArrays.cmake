#
# A function to help use Djinni JNI-Arrays with your project.
#
# FMI see Djinni support-lib/cmake/DjinniJNI.cmake
#
# This function simply adds JNI-Arrays dependencies to
# the variables that DJINNI_JNI_EXPORT_VARS() exports.
#

function(DJINNI_JNI_ARRAYS_EXPORT_VARS)

  if(NOT DEFINED DJINNI_JNI_INCLUDE_DIRS OR
     NOT DEFINED DJINNI_JNI_SRCS)
     message(SEND_ERROR "Did you run DJINNI_JNI_EXPORT_VARS() ?")
  endif()

  set(arrays_include_dirs
    build/generated-src/jni/ 
    build/generated-src/cpp/
    src/cpp/)

  set(DJINNI_JNI_INCLUDE_DIRS
    ${DJINNI_JNI_INCLUDE_DIRS}
    ${arrays_include_dirs}
    PARENT_SCOPE)

  file(GLOB_RECURSE arrays_srcs
    build/generated-src/jni/*.cpp
    build/generated-src/cpp/*.cpp
    src/cpp/*.cpp) 

  set(DJINNI_JNI_SRCS
    ${DJINNI_JNI_SRCS}
    ${arrays_srcs}
    PARENT_SCOPE)

endfunction()

