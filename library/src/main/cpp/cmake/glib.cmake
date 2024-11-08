include(ExternalProject)

ExternalProject_Add(ep_glib
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL https://download.gnome.org/sources/glib/2.83/glib-2.83.0.tar.xz
    URL_HASH SHA256=a07d9e1a57a4279c5ece71c26dc44eea12bd518ea9ff695d53e722997032b614
    DEPENDS ep_zlib ep_ffi ep_iconv
    CONFIGURE_COMMAND
    ${Meson_EXECUTABLE} setup ${EP_MESON_ARGS}
        <BINARY_DIR> <SOURCE_DIR>
        -D selinux=disabled
        -D glib_debug=disabled
        -Dlibelf=disabled
        -Dintrospection=disabled
        -Dtests=false
    BUILD_COMMAND
        ${Meson_EXECUTABLE} compile -C <BINARY_DIR>
    INSTALL_COMMAND
        ${Meson_EXECUTABLE} install -C <BINARY_DIR>
    USES_TERMINAL_DOWNLOAD true
    USES_TERMINAL_BUILD true
)
