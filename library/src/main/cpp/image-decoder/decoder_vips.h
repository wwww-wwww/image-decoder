//
// Created by len on 23/12/20.
//

#ifndef IMAGEDECODER_DECODER_VIPS_H
#define IMAGEDECODER_DECODER_VIPS_H

#include "lcms2.h"
#include "rect.h"
#include "stream.h"
#include <memory>
#include <stdio.h>

#include <vips/vips8>

class VipsDecoder {
public:
  /**
   * @brief Construct a new VipsDecoder object.
   *
   * @param stream The input stream for reading image data.
   * @param cropBorders Indicates whether ImageInfo.bounds should exclude
   *        borders around the image.
   * @param targetProfile The target color profile for the image.
   */
  VipsDecoder(std::shared_ptr<Stream>&& stream, bool cropBorders,
              cmsHPROFILE targetProfile);

  ~VipsDecoder() {
    if (targetProfile)
      cmsCloseProfile(targetProfile);
  }

  /**
   * @brief Decode a region of the image into the provided buffer.
   *
   * Decodes a region of the image specified by `srcRegion` into the given
   * buffer `outPixels`. The buffer must be pre-allocated and expect pixel data
   * in the RGBA8888 format, with a size of `outRect.width * outRect.height *
   * 4`.
   *
   * @param[out] outPixels Pointer to the output buffer for decoded pixel data.
   * @param[in] outRect Specifies the region of the resized image to decode.
   * @param[in] sampleSize Downscaling factor for resizing the image.
   */
  void decode(uint8_t* outPixels, Rect outRect, uint32_t sampleSize);

  // The bounds of the image. If `cropBorders` is true, the bounds exclude
  // borders around the image.
  Rect bounds;

private:
  // The input stream for reading image data.
  std::shared_ptr<Stream> stream;

  // The target color profile for the decoded image.
  // TODO: currently unused, keeping until color management is implemented.
  cmsHPROFILE targetProfile;

  // The VImage object. Have a reference to `stream`, so it must be declared
  // after it, to ensure it is destroyed first.
  vips::VImage image;
};

VipsDecoder* try_vips_decoder(std::shared_ptr<Stream>& stream, bool cropBorders,
                              cmsHPROFILE targetProfile);

#endif // IMAGEDECODER_DECODER_VIPS_H
