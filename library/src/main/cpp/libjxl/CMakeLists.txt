FetchContent_Declare(libjxl
  GIT_REPOSITORY  https://github.com/libjxl/libjxl
  GIT_TAG         63a481adeb6cba94c726110a5643e9d06729e8e6
  BINARY_DIR      build
  SUBBUILD_DIR    subbuild
)

set(JPEGXL_ENABLE_TOOLS OFF CACHE INTERNAL
    "Build JPEGXL user tools: cjxl and djxl.")
set(JPEGXL_ENABLE_MANPAGES OFF CACHE INTERNAL
    "Build and install man pages for the command-line tools.")
set(JPEGXL_ENABLE_BENCHMARK OFF CACHE INTERNAL
    "Build JPEGXL benchmark tools.")
set(JPEGXL_ENABLE_EXAMPLES OFF CACHE INTERNAL
    "Build JPEGXL library usage examples.")
set(JPEGXL_ENABLE_JNI OFF CACHE INTERNAL
    "Build JPEGXL JNI Java wrapper, if Java dependencies are installed.")
set(JPEGXL_ENABLE_SJPEG OFF CACHE INTERNAL
    "Build JPEGXL with support for encoding with sjpeg.")
set(JPEGXL_ENABLE_OPENEXR OFF CACHE INTERNAL
    "Build JPEGXL with support for OpenEXR if available.")
set(JPEGXL_ENABLE_SKCMS OFF CACHE INTERNAL
    "Build with skcms instead of lcms2.")
set(JPEGXL_BUNDLE_SKCMS OFF CACHE INTERNAL
    "When building with skcms, bundle it into libjxl.a.")

option(BUILD_TESTING "" OFF)

FetchContent_MakeAvailable(libjxl)

target_include_directories(imagedecoder PRIVATE
    ${libjxl_BINARY_DIR}
    ${libjxl_SOURCE_DIR}
    $<TARGET_PROPERTY:lcms2,INCLUDE_DIRECTORIES>
)
target_link_libraries(imagedecoder jxl_dec-static jxl_threads-static lcms2)
