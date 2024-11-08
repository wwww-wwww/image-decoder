include(ExternalProject)

ExternalProject_Add(ep_iconv
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.17.tar.gz
    CONFIGURE_COMMAND
        <SOURCE_DIR>/configure ${EP_AUTOTOOLS_ARGS}
        --enable-extra-encodings
    BUILD_COMMAND
        ${Make_EXECUTABLE}
    INSTALL_COMMAND
        ${Make_EXECUTABLE} install
    USES_TERMINAL_DOWNLOAD true
    USES_TERMINAL_BUILD true
)
