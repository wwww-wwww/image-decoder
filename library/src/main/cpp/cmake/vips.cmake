include(ExternalProject)

ExternalProject_Add(ep_vips
        GIT_REPOSITORY https://github.com/libvips/libvips.git
        #GIT_TAG v8.15.3
        GIT_TAG 50caa1922fa88e02c8cbfb3192fe8282d7da6eee
        DEPENDS ep_expat ep_glib ep_heif ep_highway ep_jxl ep_spng ep_webp ep_tiff ep_jpeg-turbo ep_little-cms
        PATCH_COMMAND patch -p 1 < ${CMAKE_CURRENT_LIST_DIR}/patches/vips-disable-unneded-targets.patch || true
        CONFIGURE_COMMAND
        ${Meson_EXECUTABLE} setup ${EP_MESON_ARGS}
            -Ddeprecated=false
            -Dexamples=false
            -Dcplusplus=true
            -Ddoxygen=false
            -Dgtk_doc=false
            -Dmodules=disabled
            -Dintrospection=disabled
            -Dvapi=false
            -Dcfitsio=disabled
            -Dcgif=enabled
            -Dexif=disabled
            -Dfftw=disabled
            -Dfontconfig=disabled
            -Darchive=disabled
            -Dheif=enabled
            -Dheif-module=disabled
            -Dimagequant=disabled
            -Djpeg=enabled
            -Djpeg-xl=enabled
            -Djpeg-xl-module=disabled
            -Dlcms=enabled
            -Dmagick=disabled
            -Dmatio=disabled
            -Dnifti=disabled
            -Dopenexr=disabled
            -Dopenjpeg=disabled
            -Dopenslide=disabled
            -Dhighway=enabled
            -Dorc=disabled
            -Dpangocairo=disabled
            -Dpdfium=disabled
            -Dpng=disabled
            -Dpoppler=disabled
            -Dquantizr=disabled
            -Drsvg=disabled
            -Dspng=enabled
            -Dtiff=enabled
            -Dwebp=enabled
            <BINARY_DIR> <SOURCE_DIR>
        BUILD_COMMAND
            ${Ninja_EXECUTABLE} -C <BINARY_DIR>
        INSTALL_COMMAND
            ${Ninja_EXECUTABLE} -C <BINARY_DIR> install
        USES_TERMINAL_DOWNLOAD true
        USES_TERMINAL_BUILD true
)
