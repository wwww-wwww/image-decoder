find_program(Make_EXECUTABLE make)
if(NOT Make_EXECUTABLE)
    message(FATAL_ERROR "Make is required")
endif()

find_program(Ninja_EXECUTABLE ninja)
if(NOT Ninja_EXECUTABLE)
    message(FATAL_ERROR "Ninja is required")
endif()

find_program(Meson_EXECUTABLE meson)
if(NOT Meson_EXECUTABLE)
    message(FATAL_ERROR "Meson is required")
endif()

cmake_minimum_required(VERSION 3.14)
project(imagedecoder_super C CXX ASM)

# execute_process(COMMAND "${CMAKE_COMMAND}" "-E" "environment")
#
# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()
# message(FATAL_ERROR "STOP")
#
# set(CMAKE_BUILD_TYPE Release)

set(THIRD_PARTY_LIB_PATH ${CMAKE_BINARY_DIR}/fakeroot)
set(CMAKE_FIND_ROOT_PATH ${THIRD_PARTY_LIB_PATH};${CMAKE_FIND_ROOT_PATH})
set_directory_properties(PROPERTIES EP_PREFIX ${THIRD_PARTY_LIB_PATH})

# Prefer static libraries
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so")

# Enable ccache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set_property(GLOBAL PROPERTY CMAKE_C_COMPILER_LAUNCHER ccache)
  set_property(GLOBAL PROPERTY CMAKE_C_LINKER_LAUNCHER ccache)
endif(CCACHE_FOUND)

# Set up variables for different architectures
if(ANDROID_ABI STREQUAL "arm64-v8a")
    set(ANDROID_CPU_FAMILY "aarch64")
    set(ANDROID_CPU "aarch64")
    set(ANDROID_CPU_ENDIANESS "little")
    set(ANDROID_TARGET "aarch64-linux-android")
elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
    set(ANDROID_CPU_FAMILY "arm")
    set(ANDROID_CPU "armv7-a")
    set(ANDROID_CPU_ENDIANESS "little")
    set(ANDROID_TARGET "armv7a-linux-androideabi")
elseif(ANDROID_ABI STREQUAL "x86")
    set(ANDROID_CPU_FAMILY "x86")
    set(ANDROID_CPU "i686")
    set(ANDROID_CPU_ENDIANESS "little")
    set(ANDROID_TARGET "i686-linux-android")
elseif(ANDROID_ABI STREQUAL "x86_64")
    set(ANDROID_CPU_FAMILY "x86_64")
    set(ANDROID_CPU "x86_64")
    set(ANDROID_CPU_ENDIANESS "little")
    set(ANDROID_TARGET "x86_64-linux-android")
else()
    message(FATAL_ERROR "Unsupported architecture: ${ANDROID_ABI}")
endif()

# Define the template for the cross-file
set(CROSS_FILE_CONTENT "
[host_machine]
system = 'android'
cpu_family = '${ANDROID_CPU_FAMILY}'
cpu = '${ANDROID_CPU}'
endian = '${ANDROID_CPU_ENDIANESS}'

[binaries]
c = ['${CMAKE_C_COMPILER}', '--target=${ANDROID_TARGET}${ANDROID_PLATFORM_LEVEL}']
cpp = ['${CMAKE_CXX_COMPILER}', '--target=${ANDROID_TARGET}${ANDROID_PLATFORM_LEVEL}']
ar = '${CMAKE_AR}'
ld = '${CMAKE_LINKER}'
strip = '${CMAKE_STRIP}'
ranlib = '${CMAKE_RANLIB}'
as = '${CMAKE_ASM_COMPILER}'
# pkg-config = ['sh', '${CMAKE_CURRENT_LIST_DIR}/pkg-config-wrapper.sh', '${THIRD_PARTY_LIB_PATH}']
pkg-config = 'pkg-config'

[built-in options]
c_args = ['-I${THIRD_PARTY_LIB_PATH}/include', '-Wno-error=format-nonliteral']
cpp_args = ['-I${THIRD_PARTY_LIB_PATH}/include', '-Wno-error=format-nonliteral']
c_link_args = ['-L${THIRD_PARTY_LIB_PATH}/lib']
cpp_link_args = ['-L${THIRD_PARTY_LIB_PATH}/lib']

[properties]
# sys_root = '${CMAKE_SYSROOT}'
cmake_toolchain_file = '${CMAKE_TOOLCHAIN_FILE}'
pkg_config_libdir = '${THIRD_PARTY_LIB_PATH}/lib/pkgconfig'

[cmake]
CMAKE_BUILD_WITH_INSTALL_RPATH     = 'ON'
CMAKE_FIND_ROOT_PATH_MODE_PROGRAM  = 'NEVER'
CMAKE_FIND_ROOT_PATH_MODE_LIBRARY  = 'ONLY'
CMAKE_FIND_ROOT_PATH_MODE_INCLUDE  = 'ONLY'
CMAKE_FIND_ROOT_PATH_MODE_PACKAGE  = 'ONLY'
")

# Generate the cross-file
set(MESON_CROSS_FILE_PATH "${CMAKE_BINARY_DIR}/android-cross-file.txt")
file(WRITE ${MESON_CROSS_FILE_PATH} "${CROSS_FILE_CONTENT}")

# Common args to all Mason ExternalProjects
set(EP_MESON_ARGS "--cross-file=${MESON_CROSS_FILE_PATH}"
  "--prefix=<INSTALL_DIR>"
  "--libdir=lib"
  "--default-library=static"
  "--buildtype=release"
)

# Escape ';'
string(REPLACE ";" "$<SEMICOLON>" CMAKE_FIND_ROOT_PATH_STR "${CMAKE_FIND_ROOT_PATH}")
string(REPLACE ";" "$<SEMICOLON>" CMAKE_FIND_LIBRARY_SUFFIXES_STR "${CMAKE_FIND_LIBRARY_SUFFIXES}")

# Common args to all CMake ExternalProjects
set(EP_CMAKE_ARGS "-DANDROID_ABI=${ANDROID_ABI}"
  "-DANDROID_PLATFORM=${ANDROID_PLATFORM}"
  "-DANDROID_USE_LEGACY_TOOLCHAIN_FILE=OFF"
  "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
  "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/fakeroot"
  "-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
  "-DCMAKE_SYSROOT=${CMAKE_SYSROOT}"
  "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
  "-DCMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH_STR}"
  "-DCMAKE_FIND_LIBRARY_SUFFIXES=${CMAKE_FIND_LIBRARY_SUFFIXES_STR}"
  "-DCMAKE_FIND_DEBUG_MODE=OFF"
  "-DBUILD_SHARED_LIBS=OFF"
  "-DBUILD_TESTING=OFF"
  "-DCMAKE_C_COMPILER_LAUNCHER=${CMAKE_C_COMPILER_LAUNCHER}"
  "-DCMAKE_C_LINKER_LAUNCHER=${CMAKE_C_LINKER_LAUNCHER}")

# Common args to all autotools ExternalProjects
set(EP_AUTOTOOLS_ARGS "--host=${ANDROID_TARGET}${ANDROID_PLATFORM_LEVEL}"
  "CC=${CMAKE_C_COMPILER} --target=${ANDROID_TARGET}${ANDROID_PLATFORM_LEVEL}"
  "CXX=${CMAKE_CXX_COMPILER} --target=${ANDROID_TARGET}${ANDROID_PLATFORM_LEVEL}"
  "AR=${CMAKE_AR}"
  "AS=${CMAKE_ASM_COMPILER}"
  "LD=${CMAKE_LINKER}"
  "RANLIB=${CMAKE_RANLIB}"
  "STRIP=${CMAKE_STRIP}"
  "--prefix=${THIRD_PARTY_LIB_PATH}"
  "--disable-shared"
  "--enable-static"
  "--with-pic")

include("cmake/little-cms.cmake")
include("cmake/jpeg-turbo.cmake")
include("cmake/webp.cmake")
include("cmake/zlib.cmake")
include("cmake/dav1d.cmake")
include("cmake/de265.cmake")
include("cmake/heif.cmake")
include("cmake/highway.cmake")
include("cmake/brotli.cmake")
include("cmake/jxl.cmake")
include("cmake/tiff.cmake")
include("cmake/ffi.cmake")
include("cmake/iconv.cmake")
include("cmake/glib.cmake")
include("cmake/expat.cmake")
include("cmake/spng.cmake")
include("cmake/vips.cmake")

include(ExternalProject)
ExternalProject_Add(ep_image-decoder
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/image-decoder
    DEPENDS ep_vips
    CMAKE_ARGS ${EP_CMAKE_ARGS}
      "-DBUILD_SHARED_LIBS=ON"
      "-DBUILD_STATIC_LIBS=OFF"
      "-DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=BOTH"
      "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
      "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
      "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    USES_TERMINAL_DOWNLOAD true
    USES_TERMINAL_BUILD true
)
