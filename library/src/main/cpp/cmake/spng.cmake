include(ExternalProject)

ExternalProject_Add(ep_spng
        GIT_REPOSITORY https://github.com/randy408/libspng.git
        GIT_TAG v0.7.4
        DEPENDS ep_zlib
        CMAKE_ARGS
            ${EP_CMAKE_ARGS}
            -DZLIB_ROOT:STRING=${CMAKE_BINARY_DIR}/fakeroot
        USES_TERMINAL_DOWNLOAD true
        USES_TERMINAL_BUILD true
)
