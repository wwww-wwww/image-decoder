cmake_minimum_required(VERSION 3.14)
project(imagedecoder C CXX ASM)

option(WITH_JPEG "Include JPEG decoder" ON)
option(WITH_PNG "Include PNG decoder" ON)
option(WITH_WEBP "Include WebP decoder" ON)
option(WITH_HEIF "Include HEIF decoder" ON)
option(WITH_AVIF "Include AVIF decoder" ON)
option(WITH_JXL "Include JXL decoder" ON)
option(WITH_VIPS "Include libvips and use it decoders" ON)

add_library(imagedecoder SHARED
  java_stream.cpp
  java_wrapper.cpp
  java_objects.cpp
  borders.cpp
  row_convert.cpp
)

target_link_libraries(imagedecoder android jnigraphics log lcms2)

if(WITH_JPEG)
  add_definitions(-DHAVE_LIBJPEG)
  target_sources(imagedecoder PRIVATE decoder_jpeg.cpp)
  target_link_libraries(imagedecoder jpeg)
endif()
if(WITH_PNG)
  add_definitions(-DHAVE_LIBPNG)
  target_sources(imagedecoder PRIVATE decoder_png.cpp)
  target_link_libraries(imagedecoder png)
endif()
if(WITH_WEBP OR WITH_HEIF OR WITH_AVIF)
  add_definitions(-DHAVE_LIBWEBP)
  target_sources(imagedecoder PRIVATE decoder_webp.cpp)
  target_link_libraries(imagedecoder webpdecoder webpdemux z)
endif()
if(WITH_HEIF OR WITH_AVIF)
  add_definitions(-DHAVE_LIBHEIF)
  target_sources(imagedecoder PRIVATE decoder_heif.cpp)
  target_link_libraries(imagedecoder heif dav1d de265)
endif()
if(WITH_JXL)
  add_definitions(-DHAVE_LIBJXL)
  target_sources(imagedecoder PRIVATE decoder_jxl.cpp)
  target_link_libraries(imagedecoder jxl_threads jxl_cms jxl brotlidec brotlienc brotlicommon hwy -Wl,--allow-multiple-definition)
endif()
if(WITH_VIPS)
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
endif()
