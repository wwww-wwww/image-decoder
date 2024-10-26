include(ExternalProject)

ExternalProject_Add(ep_png
    GIT_REPOSITORY  https://github.com/glennrp/libpng
    GIT_TAG         v1.6.42
    CMAKE_ARGS
        ${EP_CMAKE_ARGS}
        -DPNG_SHARED=OFF
        -DPNG_TOOLS=ON
        -DPNG_TESTS=OFF
    USES_TERMINAL_DOWNLOAD true
    USES_TERMINAL_BUILD true
    GIT_SHALLOW true
)
