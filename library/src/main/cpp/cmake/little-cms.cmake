include(ExternalProject)

ExternalProject_Add(ep_little-cms
    GIT_REPOSITORY  https://github.com/mm2/Little-CMS
    GIT_TAG         lcms2.16
    # DOWNLOAD_COMMAND git clone https://github.com/mm2/Little-CMS --branch lcms2.16 --depth 1 ep_lcms || true
    CONFIGURE_COMMAND
        <SOURCE_DIR>/autogen.sh ${EP_AUTOTOOLS_ARGS}
    BUILD_COMMAND
        ${Make_EXECUTABLE} all
    INSTALL_COMMAND
        ${Make_EXECUTABLE} install

    # autogen.sh for example calls `aclocal -I m4` which only works in the source directory
    BUILD_IN_SOURCE true 

    USES_TERMINAL_DOWNLOAD true
    USES_TERMINAL_BUILD true
    GIT_SHALLOW true
)
