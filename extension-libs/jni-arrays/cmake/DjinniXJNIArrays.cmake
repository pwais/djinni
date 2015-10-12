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
     NOT DEFINED DJINNI_JNI_SRCS OR
	 NOT DEFINED DJINNI_ROOT)
     message(SEND_ERROR "Did you run DJINNI_JNI_EXPORT_VARS() ?")
  endif()

  set(jni_arrays_root ${DJINNI_ROOT}/extension-libs/jni-arrays/)

  set(arrays_include_dirs
    ${jni_arrays_root}/build/generated-src/jni/ 
    ${jni_arrays_root}/build/generated-src/cpp/
    ${jni_arrays_root}/src/cpp/)

  list(APPEND DJINNI_JNI_INCLUDE_DIRS ${arrays_include_dirs})
  set(DJINNI_JNI_INCLUDE_DIRS ${DJINNI_JNI_INCLUDE_DIRS} PARENT_SCOPE)

  file(GLOB_RECURSE arrays_srcs
    ${jni_arrays_root}/build/generated-src/jni/*.cpp
    ${jni_arrays_root}/build/generated-src/cpp/*.cpp
    ${jni_arrays_root}/src/cpp/*.cpp) 

  set(DJINNI_JNI_SRCS ${DJINNI_JNI_SRCS} ${arrays_srcs} PARENT_SCOPE)

endfunction()

