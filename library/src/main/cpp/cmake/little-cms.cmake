include(ExternalProject)

ExternalProject_Add(ep_little-cms
    GIT_REPOSITORY  https://github.com/mm2/Little-CMS
    GIT_TAG         lcms2.16
    # DOWNLOAD_COMMAND git clone https://github.com/mm2/Little-CMS --branch lcms2.16 --depth 1 ep_lcms || true
    CONFIGURE_COMMAND
    <SOURCE_DIR>/autogen.sh --host ${CMAKE_LIBRARY_ARCHITECTURE}
    "CC=${CMAKE_C_COMPILER} --target=${CMAKE_C_COMPILER_TARGET}"
    "CXX=${CMAKE_CXX_COMPILER} --target=${CMAKE_CXX_COMPILER_TARGET}"
    "AR=${CMAKE_AR}"
    "AS=${CMAKE_ASM_COMPILER}"
    "LD=${CMAKE_LINKER}"
    "RANLIB=${CMAKE_RANLIB}"
    "STRIP=${CMAKE_STRIP}"
    --prefix ${CMAKE_BINARY_DIR}/fakeroot
    BUILD_COMMAND
        ${Make_EXECUTABLE} all
    INSTALL_COMMAND
        ${Make_EXECUTABLE} install

    # autogen.sh for example calls `aclocal -I m4` which only works in the source directory
    BUILD_IN_SOURCE true 

    USES_TERMINAL_DOWNLOAD true
    USES_TERMINAL_BUILD true
    GIT_PROGRESS true
    GIT_SHALLOW true
)
