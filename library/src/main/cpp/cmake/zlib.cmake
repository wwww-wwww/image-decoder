include(ExternalProject)

ExternalProject_Add(ep_zlib
    GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng.git
    GIT_TAG 2.2.1
    UPDATE_DISCONNECTED True
    CMAKE_ARGS
        ${EP_CMAKE_ARGS}
        -DINSTALL_PKGCONFIG_DIR=${THIRD_PARTY_LIB_PATH}/lib/pkgconfig
        -DZLIB_COMPAT=ON
        -DZLIB_ENABLE_TESTS=OFF
    USES_TERMINAL_DOWNLOAD true
    USES_TERMINAL_BUILD true
)
