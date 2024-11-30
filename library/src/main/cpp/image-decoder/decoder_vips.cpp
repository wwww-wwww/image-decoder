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
    : stream(std::move(stream)), cropBorders(cropBorders),
      targetProfile(targetProfile) {

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

  this->info = parseInfo();
}

void VipsDecoder::decode(uint8_t* outPixels, const Rect outRect,
                         const uint32_t sampleSize) {

  double scale = 1.0 / sampleSize;
  VImage resized = image.resize(
      scale, VImage::option()->set("kernel", VIPS_KERNEL_LANCZOS3));

  // convert image to sRGB. We could just pass the targetProfile here, but
  // libvips only support loading it from a file. See:
  // https://github.com/libvips/libvips/issues/4283
  resized = resized.icc_transform("srgb");

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
    memcpy(outline, line, outRect.width * 4);
    outline += outRect.width * 4;
  }
  // ensure we didn't write past the end of the buffer
  assert(outline <= outPixelsEnd);
}

ImageInfo VipsDecoder::parseInfo() {
  uint32_t imageWidth = this->image.width();
  uint32_t imageHeight = this->image.height();

  Rect bounds = {.x = 0, .y = 0, .width = imageWidth, .height = imageHeight};

  // Crop the image if `cropBorders` is enabled
  if (this->cropBorders) {
    // convert image to gray
    VImage gray_image = image.colourspace(VIPS_INTERPRETATION_B_W);
    bounds = findBorders((uint8_t*)gray_image.data(), imageWidth, imageHeight);
  }

  return ImageInfo{
      .imageWidth = imageWidth,
      .imageHeight = imageHeight,
      .isAnimated = false,
      .bounds = bounds,
  };
}
