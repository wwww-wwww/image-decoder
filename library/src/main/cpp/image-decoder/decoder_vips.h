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

struct ImageInfo {
  uint32_t imageWidth;
  uint32_t imageHeight;
  bool isAnimated;
  Rect bounds;
};

class VipsDecoder {
public:
  VipsDecoder(std::shared_ptr<Stream>&& stream, bool cropBorders,
              cmsHPROFILE targetProfile);

  void decode(uint8_t* outPixels, Rect outRect, Rect srcRegion,
              uint32_t sampleSize);

protected:
  std::shared_ptr<Stream> stream; ///< The input stream for reading image data.

public:
  bool cropBorders; ///< Indicates whether to crop borders around the image.
  cmsHPROFILE targetProfile = nullptr; ///< The color profile for decoding.
  ImageInfo info; ///< Metadata about the image being decoded.
  cmsHTRANSFORM transform = nullptr; ///< A color transformation.
  bool useTransform =
      false; ///< Whether the color transformation should be applied.
  cmsUInt32Number inType; ///< Input type for the color transformation.

private:
  vips::VImage image;
  ImageInfo parseInfo();
};

VipsDecoder* try_vips_decoder(std::shared_ptr<Stream>& stream, bool cropBorders,
                              cmsHPROFILE targetProfile);

#endif // IMAGEDECODER_DECODER_VIPS_H
