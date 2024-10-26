include(ExternalProject)

ExternalProject_Add(ep_png
    GIT_REPOSITORY  https://github.com/glennrp/libpng
    GIT_TAG         v1.6.42
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/fakeroot
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DANDROID_ABI=${ANDROID_ABI}
        -DANDROID_PLATFORM=${ANDROID_PLATFORM}
        -DANDROID_USE_LEGACY_TOOLCHAIN_FILE=OFF
        -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
        -DCMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}

        -DPNG_SHARED=OFF
        -DPNG_TOOLS=ON
        -DPNG_TESTS=OFF
    USES_TERMINAL_DOWNLOAD true
    USES_TERMINAL_BUILD true
    GIT_PROGRESS true
    GIT_SHALLOW true
)


# include(ExternalProject)
#
# ExternalProject_Add(ep_png
#     GIT_REPOSITORY  https://github.com/glennrp/libpng
#     GIT_TAG         v1.6.42
#     CONFIGURE_COMMAND
#         <SOURCE_DIR>/configure ${HOST_FLAG}
#         --prefix ${CMAKE_BINARY_DIR}/fakeroot
#     BUILD_COMMAND
#         ${Make_EXECUTABLE} all
#     INSTALL_COMMAND
#         ${Make_EXECUTABLE} install
#
#     USES_TERMINAL_DOWNLOAD true
#     USES_TERMINAL_BUILD true
#     GIT_PROGRESS true
#     GIT_SHALLOW true
# )
