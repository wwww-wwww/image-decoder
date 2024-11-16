//
// Created by len on 23/12/20.
//

#ifndef IMAGEDECODER_DECODER_BASE_H
#define IMAGEDECODER_DECODER_BASE_H

#include "borders.h"
#include "java_stream.h"
#include <lcms2.h>
#include <vector>

struct ImageInfo {
  uint32_t imageWidth;  ///< Width of the image in pixels.
  uint32_t imageHeight; ///< Height of the image in pixels.
  bool isAnimated;      ///< Indicates if the image is an animation.
  Rect bounds;          ///< Bounds of the image. Indicates the cropped region.
};

class BaseDecoder {
public:
  /**
   * @brief Constructs a new decoder for processing image data.
   *
   * @param[in] stream A shared pointer to the input stream from which image
   *            data will be read.
   * @param[in] cropBorders If true, the decoder will detect and crop borders
   *            around the image.
   * @param[in] targetProfile The color space profile to decode the image into.
   */
  BaseDecoder(std::shared_ptr<Stream>&& stream, bool cropBorders,
              cmsHPROFILE targetProfile) {
    this->stream = std::move(stream);
    this->cropBorders = cropBorders;
    this->targetProfile = targetProfile;
  }
  virtual ~BaseDecoder() {
    if (transform) {
      cmsDeleteTransform(transform);
    }
    if (targetProfile) {
      cmsCloseProfile(targetProfile);
    }
  };

  /**
   * @brief Decodes the data in `stream` to an image and stores the result in
   * `outPixels`.
   *
   * The decoding process uses the color profile specified by
   * `this->targetProfile`. Only the pixels within the bounds of `inRect` will
   * be decoded.
   *
   * This will may be called multiple times to decode different regions of the
   * same image.
   *
   * @param[out] outPixels The buffer where the decoded image will be stored.
   * @param[in] outRect Specifies the dimensions to which the decoded region
   *            will be scaled to. The position of `outRect` is unused.
   * @param[in] inRect The region of the image to decode, in the coordinate
   *            space of the image after cropping, if `this->cropBorders` is
   *            true.
   * @param[in] sampleSize The scale between `inRect` and `outRect`. Commonly a
   *            power of 2.
   */
  virtual void decode(uint8_t* outPixels, Rect outRect, Rect inRect,
                      uint32_t sampleSize) = 0;

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
};

#endif // IMAGEDECODER_DECODER_BASE_H
