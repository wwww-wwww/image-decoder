include(ExternalProject)

ExternalProject_Add(ep_tiff
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL http://download.osgeo.org/libtiff/tiff-4.7.0.tar.gz
    DEPENDS ep_zlib ep_webp ep_jpeg-turbo
    PATCH_COMMAND patch -p 1 < ${CMAKE_CURRENT_LIST_DIR}/patches/tiff-0001-cmake--Replace-CMath--CMath-with-direct-link-to-avoid-export-of-target.patch || true
    CMAKE_ARGS
        ${EP_CMAKE_ARGS}
        -Djbig=OFF
        -Dlzma=OFF
        -Dlerc=OFF
        -Dlibdeflate=OFF
        -Dcxx=OFF
        -Djpeg=ON
        -Dtiff-tools=OFF
        -Dtiff-tests=OFF
        -Dtiff-contrib=OFF
        -Dtiff-docs=OFF
    USES_TERMINAL_DOWNLOAD true
    USES_TERMINAL_BUILD true
)
