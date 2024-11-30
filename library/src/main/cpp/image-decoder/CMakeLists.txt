cmake_minimum_required(VERSION 3.14)
project(imagedecoder C CXX ASM)

add_library(imagedecoder SHARED
  java_stream.cpp
  java_wrapper.cpp
  java_objects.cpp
  borders.cpp
  row_convert.cpp
)

target_link_libraries(imagedecoder android jnigraphics log lcms2)

# set pkg-config libdir path
set(ENV{PKG_CONFIG_LIBDIR} "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
message(STATUS "PKG_CONFIG_LIBDIR=${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")

find_package(PkgConfig REQUIRED)

pkg_search_module(VIPS REQUIRED vips-cpp)
target_include_directories(imagedecoder PRIVATE ${VIPS_STATIC_INCLUDE_DIRS})
target_link_directories(imagedecoder PRIVATE ${VIPS_STATIC_LIBRARY_DIRS})

# remove `spng` from VIPS_STATIC_LIBRARY_DIRS, and add `spng_static`
list(REMOVE_ITEM VIPS_STATIC_LIBRARIES "spng")
list(APPEND VIPS_STATIC_LIBRARIES "spng_static")

list(REMOVE_ITEM VIPS_STATIC_LIBRARIES "c++")

message(STATUS "VIPS_STATIC_INCLUDE_DIRS=${VIPS_STATIC_INCLUDE_DIRS}")
message(STATUS "VIPS_STATIC_LIBRARY_DIRS=${VIPS_STATIC_LIBRARY_DIRS}")
message(STATUS "VIPS_STATIC_LIBRARIES=${VIPS_STATIC_LIBRARIES}")

add_definitions(-DHAVE_LIBVIPS)
target_sources(imagedecoder PRIVATE decoder_vips.cpp)
target_link_libraries(imagedecoder vips-cpp ${VIPS_STATIC_LIBRARIES} -static-libgcc -static-libstdc++)
