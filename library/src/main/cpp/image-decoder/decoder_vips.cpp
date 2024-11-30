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
                         cmsHPROFILE targetProfile) {

  this->stream = std::move(stream);
  this->cropBorders = cropBorders;
  this->targetProfile = targetProfile;

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
                         const Rect inRect, const uint32_t sampleSize) {
  // Crop the image. Make sure to not mutate the original image.
  VImage cropped = image.crop(inRect.x, inRect.y, inRect.width, inRect.height);

  double scale = 1.0 / sampleSize;

  cropped = cropped.resize(
      scale, VImage::option()->set("kernel", VIPS_KERNEL_LANCZOS3));

  if (cropped.width() != outRect.width || cropped.height() != outRect.height) {
    // The resize will round the size to the nearest integer, but we outRect is
    // always inRect diveded by sampleSize rounded down, so any size mismatch is
    // due to round up, which we can fix by cropping the image down.
    cropped = cropped.crop(0, 0, outRect.width, outRect.height);
  }

  uint32_t croppedWidth = cropped.width();
  uint32_t croppedHeight = cropped.height();

  // convert image to sRGB. We could just pass the targetProfile here, but
  // libvips only support loading it from a file. See:
  // https://github.com/libvips/libvips/issues/4283
  cropped = cropped.icc_transform("srgb");
  auto srcProfile = cmsCreate_sRGBProfile();

  // convert to RGBA8888
  cropped = cropped.cast(VIPS_FORMAT_UCHAR);
  if (!cropped.has_alpha()) {
    cropped = cropped.addalpha();
  }

  // Write the cropped data into the provided buffer
  const VRegion region = cropped.region(0, 0, croppedWidth, croppedHeight);
  const size_t stride = region.stride();
  const uint8_t* outPixelsEnd = outPixels + outRect.width * outRect.height * 4;
  uint8_t* outline = outPixels;
  for (uint32_t y = 0; y < croppedHeight; y++) {
    const uint8_t* line = region.addr(0, y);
    memcpy(outline, line, outRect.width * 4);
    outline += stride;
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
