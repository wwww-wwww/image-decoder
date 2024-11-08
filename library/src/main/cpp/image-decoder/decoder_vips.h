//
// Created by len on 23/12/20.
//

#ifndef IMAGEDECODER_DECODER_VIPS_H
#define IMAGEDECODER_DECODER_VIPS_H

#include "decoder_base.h"
#include <memory>
#include <stdio.h>

#include <vips/vips8>

class VipsDecoder : public BaseDecoder {
public:
  VipsDecoder(std::shared_ptr<Stream>&& stream, bool cropBorders,
              cmsHPROFILE targetProfile);

  void decode(uint8_t* outPixels, Rect outRect, Rect srcRegion,
              uint32_t sampleSize) override;

private:
  vips::VImage image;
  ImageInfo parseInfo();
};

VipsDecoder* try_vips_decoder(std::shared_ptr<Stream>& stream, bool cropBorders,
                              cmsHPROFILE targetProfile);

#endif // IMAGEDECODER_DECODER_VIPS_H
