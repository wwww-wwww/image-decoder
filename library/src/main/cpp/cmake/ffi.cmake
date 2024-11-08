include(ExternalProject)

ExternalProject_Add(ep_ffi
    GIT_REPOSITORY      https://github.com/libffi/libffi.git
    GIT_TAG             v3.4.6
    BUILD_IN_SOURCE true
    CONFIGURE_COMMAND
        <SOURCE_DIR>/autogen.sh && <SOURCE_DIR>/configure ${EP_AUTOTOOLS_ARGS}
        --disable-exec-static-tramp
        --disable-multi-os-directory
        --enable-pax_emutramp
    BUILD_COMMAND
        ${Make_EXECUTABLE} all
    INSTALL_COMMAND
        ${Make_EXECUTABLE} install
    USES_TERMINAL_DOWNLOAD true
    USES_TERMINAL_BUILD true
)
