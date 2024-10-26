
find_program(Make_EXECUTABLE make)
if(NOT Make_EXECUTABLE)
    message(FATAL_ERROR "Make is required")
endif()

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
project(imagedecoder C CXX ASM)
# include(FetchContent)

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

option(WITH_JPEG "Include JPEG decoder" ON)
option(WITH_PNG "Include PNG decoder" ON)
option(WITH_WEBP "Include WebP decoder" ON)
option(WITH_HEIF "Include HEIF decoder" ON)
option(WITH_AVIF "Include AVIF decoder" ON)
option(WITH_JXL "Include JXL decoder" ON)

set(THIRD_PARTY_LIB_PATH ${CMAKE_BINARY_DIR}/fakeroot)
set(CMAKE_FIND_ROOT_PATH ${THIRD_PARTY_LIB_PATH};${CMAKE_FIND_ROOT_PATH})
set_directory_properties(PROPERTIES EP_PREFIX ${THIRD_PARTY_LIB_PATH})

add_library(imagedecoder SHARED
  java_stream.cpp
  java_wrapper.cpp
  java_objects.cpp
  borders.cpp
  row_convert.cpp
)

# # Change the default path of fetch content to avoid downloading the same dependencies per architecture get_filename_component(deps "../_deps" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
# set(FETCHCONTENT_BASE_DIR ${deps})

# Set up variables for different architectures
if(ANDROID_ABI STREQUAL "arm64-v8a")
    set(ANDROID_CPU_FAMILY "aarch64")
    set(ANDROID_CPU "armv8-a")
    set(ANDROID_CPU_ENDIANESS "little")
elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
    set(ANDROID_CPU_FAMILY "arm")
    set(ANDROID_CPU "armv7-a")
    set(ANDROID_CPU_ENDIANESS "little")
elseif(ANDROID_ABI STREQUAL "x86")
    set(ANDROID_CPU_FAMILY "x86")
    set(ANDROID_CPU "i686")
    set(ANDROID_CPU_ENDIANESS "little")
elseif(ANDROID_ABI STREQUAL "x86_64")
    set(ANDROID_CPU_FAMILY "x86_64")
    set(ANDROID_CPU "x86_64")
    set(ANDROID_CPU_ENDIANESS "little")
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
c = ['${CMAKE_C_COMPILER}', '--target=${CMAKE_C_COMPILER_TARGET}']
cpp = ['${CMAKE_CXX_COMPILER}', '--target=${CMAKE_CXX_COMPILER_TARGET}']
ar = '${CMAKE_AR}'
ld = '${CMAKE_LINKER}'
strip = '${CMAKE_STRIP}'
ranlib = '${CMAKE_RANLIB}'
as = '${CMAKE_ASM_COMPILER}'

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
  "--buildtype=release")

# Common args to all CMake ExternalProjects
set(EP_CMAKE_ARGS "-DANDROID_ABI=${ANDROID_ABI}"
  "-DANDROID_PLATFORM=${ANDROID_PLATFORM}"
  "-DANDROID_USE_LEGACY_TOOLCHAIN_FILE=OFF"
  "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
  "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/fakeroot"
  "-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
  "-DCMAKE_SYSROOT=${CMAKE_SYSROOT}"
  "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
  "-DCMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}"
  "-DCMAKE_FIND_LIBRARY_SUFFIXES=${CMAKE_FIND_LIBRARY_SUFFIXES}"
  "-DCMAKE_FIND_DEBUG_MODE=OFF"
  "-DBUILD_SHARED_LIBS=OFF"
  "-DBUILD_TESTING=OFF")

# Common args to all autotools ExternalProjects
set(EP_AUTOTOOLS_ARGS "--host=${CMAKE_LIBRARY_ARCHITECTURE}"
  "CC=${CMAKE_C_COMPILER} --target=${CMAKE_LIBRARY_ARCHITECTURE}${ANDROID_PLATFORM_LEVEL}"
  "CXX=${CMAKE_CXX_COMPILER} --target=${CMAKE_LIBRARY_ARCHITECTURE}${ANDROID_PLATFORM_LEVEL}"
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
include("cmake/png.cmake")
include("cmake/webp.cmake")
include("cmake/zlib.cmake")
include("cmake/dav1d.cmake")
include("cmake/de265.cmake")
include("cmake/heif.cmake")
include("cmake/highway.cmake")
include("cmake/brotli.cmake")
include("cmake/jxl.cmake")

target_include_directories(imagedecoder PRIVATE ${THIRD_PARTY_LIB_PATH}/include)
target_link_directories(imagedecoder PRIVATE ${THIRD_PARTY_LIB_PATH}/lib)

# Prefer static libraries
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so")

add_dependencies(imagedecoder ep_little-cms)
target_link_libraries(imagedecoder android jnigraphics log lcms2)

if(WITH_JPEG)
  add_definitions(-DHAVE_LIBJPEG)
  target_sources(imagedecoder PRIVATE decoder_jpeg.cpp)
  target_link_libraries(imagedecoder jpeg)
  add_dependencies(imagedecoder ep_jpeg-turbo)
endif()
if(WITH_PNG)
  add_definitions(-DHAVE_LIBPNG)
  target_sources(imagedecoder PRIVATE decoder_png.cpp)
  target_link_libraries(imagedecoder png)
  add_dependencies(imagedecoder ep_png)
endif()
if(WITH_WEBP OR WITH_HEIF OR WITH_AVIF)
  add_definitions(-DHAVE_LIBWEBP)
  target_sources(imagedecoder PRIVATE decoder_webp.cpp)
  target_link_libraries(imagedecoder webpdecoder webpdemux z)
  add_dependencies(imagedecoder ep_webp)
endif()
if(WITH_HEIF OR WITH_AVIF)
  add_definitions(-DHAVE_LIBHEIF)
  target_sources(imagedecoder PRIVATE decoder_heif.cpp)
  target_link_libraries(imagedecoder heif dav1d de265)
  add_dependencies(imagedecoder ep_heif)
endif()
if(WITH_JXL)
  add_definitions(-DHAVE_LIBJXL)
  target_sources(imagedecoder PRIVATE decoder_jxl.cpp)
  target_link_libraries(imagedecoder jxl_threads jxl_cms jxl brotlidec brotlienc brotlicommon hwy -Wl,--allow-multiple-definition)
  add_dependencies(imagedecoder ep_jxl)
endif()
