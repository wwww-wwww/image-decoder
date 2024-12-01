#include "decoder_vips.h"

#include "borders.h"
#include "lcms2.h"
#include "log.h"

#include "stream.h"
#include "vips/resample.h"
#include <vips/vips8>

#include <cassert>

using namespace vips;

VipsDecoder* try_vips_decoder(std::shared_ptr<Stream>& stream, bool cropBorders,
                              cmsHPROFILE targetProfile) {

  try {
    return new VipsDecoder(std::move(stream), cropBorders, targetProfile);
  } catch (const std::exception& e) {
    LOGW("Failed to initialize VipsDecoder: %s", e.what());
    return nullptr;
  }
}

VipsDecoder::VipsDecoder(std::shared_ptr<Stream>&& stream, bool cropBorders,
                         cmsHPROFILE targetProfile)
    : stream(std::move(stream)), targetProfile(targetProfile) {

  if (VIPS_INIT("VipsDecoder")) {
    LOGE("Failed to initialize libvips.");
    throw std::runtime_error("libvips initialization failed");
  }

  // the VImage does not take ownership of the `stream`.
  this->image = VImage::new_from_buffer(
      this->stream->bytes, this->stream->size, nullptr,
      // we need to decode multiple regions from the image, so access must be
      // random.
      VImage::option()->set("access", VIPS_ACCESS_RANDOM));

  this->bounds = {.x = 0,
                  .y = 0,
                  .width = (uint32_t)image.width(),
                  .height = (uint32_t)image.height()};

  // Crop the image if `cropBorders` is enabled
  if (cropBorders) {
    // convert image to gray
    VImage gray_image =
        image.colourspace(VIPS_INTERPRETATION_B_W).cast(VIPS_FORMAT_UCHAR);
    this->bounds =
        findBorders((uint8_t*)gray_image.data(), image.width(), image.height());
  }
}

void VipsDecoder::decode(uint8_t* outPixels, const Rect outRect,
                         const uint32_t sampleSize) {

  double scale = 1.0 / sampleSize;
  VImage resized = image.resize(
      scale, VImage::option()->set("kernel", VIPS_KERNEL_LANCZOS3));

  cmsHTRANSFORM transform = nullptr;
  cmsHPROFILE profile;
  int bands = image.bands();

  if (resized.get_typeof(VIPS_META_ICC_NAME) != 0) {
    size_t size;
    const void* data = resized.get_blob(VIPS_META_ICC_NAME, &size);

    profile = cmsOpenProfileFromMem(data, size);
    if (profile) {
      cmsColorSpaceSignature colorspace = cmsGetColorSpace(profile);

      if ((bands > 2) && (colorspace == cmsSigRgbData)) {
        LOGI("RGB");
        transform = cmsCreateTransform(
            profile, TYPE_RGBA_8, targetProfile, TYPE_RGBA_8,
            cmsGetHeaderRenderingIntent(profile), cmsFLAGS_COPY_ALPHA);
      } else if ((bands == 4) && (colorspace == cmsSigCmykData)) {
        LOGI("CMYK");
        transform = cmsCreateTransform(
            profile, TYPE_CMYK_8, targetProfile, TYPE_RGBA_8,
            cmsGetHeaderRenderingIntent(profile), cmsFLAGS_COPY_ALPHA);
      } else {
        LOGI("n");
      }

      cmsCloseProfile(profile);
    }
  }

  if (!transform) {
    resized = resized.icc_transform("srgb");
    cmsHPROFILE profile = cmsCreate_sRGBProfile();
    transform = cmsCreateTransform(
        profile, TYPE_RGBA_8, targetProfile, TYPE_RGBA_8,
        cmsGetHeaderRenderingIntent(profile), cmsFLAGS_COPY_ALPHA);
    cmsCloseProfile(profile);
  }

  // convert to RGBA8888
  resized = resized.cast(VIPS_FORMAT_UCHAR);
  if (!resized.has_alpha()) {
    resized = resized.addalpha();
  }

  // Write the cropped data into the provided buffer
  const VRegion region =
      resized.region(outRect.x, outRect.y, outRect.width, outRect.height);
  const uint8_t* outPixelsEnd = outPixels + outRect.width * outRect.height * 4;
  uint8_t* outline = outPixels;
  for (uint32_t y = outRect.y; y < outRect.y + outRect.height; y++) {
    const uint8_t* line = region.addr(outRect.x, y);
    cmsDoTransform(transform, line, outline, outRect.width);
    outline += outRect.width * 4;
  }
  // ensure we didn't write past the end of the buffer
  assert(outline <= outPixelsEnd);
}
