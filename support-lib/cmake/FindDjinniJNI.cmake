#
# Finds the Djinni JNI Support library and headers
#
# Configuration variables (optional):
#   DJINNI_ROOT
#     Use this Djinni source root (e.g. a djinni repo checkout)
#
# Exported variables:
#   DJINNI_JNI_INCLUDE_DIRS
#     Include directories for the library, *including* JNI includes.
#   DJINNI_JNI_LIBRARIES
#     The Djinni JNI Support library path as well as (system)
#     JNI libraries.
#   DJINNI_JNI_DEFINITIONS
#     Compiler flags required for building with djinni.
#   DJINNI_JNI_FOUND
#     Was Djinni JNI Support found?
#
# Example usage:
#   find_package(DjinniJNI REQUIRED)
#   include_directories(${DJINNI_JNI_INCLUDE_DIRS})       # Includes system JNI includes
#   add_definitions(${DJINNI_JNI_DEFINITIONS})
#   target_link_libraries(mylib ${DJINNI_JNI_LIBRARIES})  # Includes system JNI libs
#

# Djinni requires JNI
find_package(JNI)
if (NOT JNI_FOUND)
  message(
    FATAL_ERROR
    "Could not find JNI. Did you install a JDK? Set $JAVA_HOME to override")
endif()

# Use pkg-config to get path hints and definitions
find_package(PkgConfig QUIET)
pkg_check_modules(PKGCONFIG_CAPNP djinni_jni)

find_library(DJINNI_SUPPORT_LIB DjinniJNISupport
  HINTS "${PKGCONFIG_DJINNI_JNI_LIBDIR}" ${PKGCONFIG_DJINNI_JNI_LIBRARY_DIRS} ${DJINNI_ROOT}/build_jni/local/lib)

set(DJINNI_JNI_LIBRARIES ${DJINNI_SUPPORT_LIB} ${JNI_LIBRARIES})

find_path(DJINNI_JNI_INCLUDE_DIRS djinni_support.hpp
  HINTS "${PKGCONFIG_DJINNI_JNI_INCLUDEDIR}" ${PKGCONFIG_DJINNI_JNI_INCLUDE_DIRS} ${DJINNI_ROOT}/build_jni/local/include)
set(DJINNI_JNI_INCLUDE_DIRS ${DJINNI_INCLUDE_DIRS} ${JNI_INCLUDE_DIRS})

# Unfortunately, we can't reference these from support-lib/CMakeLists.txt$support_common_flags
set(DJINNI_JNI_DEFINITIONS -std=c++1y)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DJINNI_JNI DEFAULT_MSG
  DJINNI_JNI_INCLUDE_DIRS
  DJINNI_JNI_LIBRARIES
  DJINNI_JNI_DEFINITIONS)


