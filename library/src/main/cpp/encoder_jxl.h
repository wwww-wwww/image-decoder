//
// Created by w on 03/05/24.
//

#ifndef IMAGEDECODER_ENCODER_JXL_H
#define IMAGEDECODER_ENCODER_JXL_H

#include "log.h"
#include <jxl/codestream_header.h>
#include <jxl/color_encoding.h>
#include <jxl/encode.h>
#include <jxl/encode_cxx.h>
#include <jxl/thread_parallel_runner.h>
#include <jxl/thread_parallel_runner_cxx.h>
#include <vector>

bool jxl_encode(const uint8_t* data, const size_t data_size,
                const uint32_t xsize, const uint32_t ysize,
                const uint8_t components, const float distance,
                const uint8_t* icc_profile, size_t icc_profile_size,
                std::vector<uint8_t>* compressed) {
  LOGW("encoder jxl_encode components %d xsize %d ysize %d distance %f icc %d",
       components, xsize, ysize, distance, icc_profile != nullptr);
  auto enc = JxlEncoderMake(/*memory_manager=*/nullptr);

  auto runner = JxlThreadParallelRunnerMake(
      /*memory_manager=*/nullptr,
      JxlThreadParallelRunnerDefaultNumWorkerThreads());
  if (JXL_ENC_SUCCESS != JxlEncoderSetParallelRunner(enc.get(),
                                                     JxlThreadParallelRunner,
                                                     runner.get())) {
    LOGW("encoder JxlEncoderSetParallelRunner failed");
    return false;
  }

  JxlPixelFormat pixel_format = {components, JXL_TYPE_UINT8, JXL_LITTLE_ENDIAN,
                                 0};

  JxlBasicInfo basic_info;
  JxlEncoderInitBasicInfo(&basic_info);

  // only 8 bit input
  basic_info.bits_per_sample = 8;
  basic_info.exponent_bits_per_sample = 0;
  basic_info.alpha_exponent_bits = 0;

  if (components < 3) {
    basic_info.num_color_channels = 1;
  } else {
    basic_info.num_color_channels = 3;
  }

  if (components == 2 || components == 4) {
    basic_info.alpha_bits = 8;
    basic_info.num_extra_channels = 1;
  } else {
    basic_info.alpha_bits = 0;
  }

  basic_info.xsize = xsize;
  basic_info.ysize = ysize;
  basic_info.uses_original_profile = distance == 0;
  if (JXL_ENC_SUCCESS != JxlEncoderSetBasicInfo(enc.get(), &basic_info)) {
    LOGW("encoder JxlEncoderSetBasicInfo failed\n");
    return false;
  }

  if (icc_profile) {
    if (JXL_ENC_SUCCESS !=
        JxlEncoderSetICCProfile(enc.get(), icc_profile, icc_profile_size)) {
      LOGW("encoder JxlEncoderSetICCProfile failed\n");
      return false;
    }
  } else {
    JxlColorEncoding color_encoding = {};
    JXL_BOOL is_gray = TO_JXL_BOOL(pixel_format.num_channels < 3);
    JxlColorEncodingSetToSRGB(&color_encoding, is_gray);

    if (JXL_ENC_SUCCESS !=
        JxlEncoderSetColorEncoding(enc.get(), &color_encoding)) {
      LOGW("encoder JxlEncoderSetColorEncoding failed\n");
      return false;
    }
  }

  JxlEncoderFrameSettings* opts =
      JxlEncoderFrameSettingsCreate(enc.get(), nullptr);

  JxlEncoderSetFrameLossless(opts, distance == 0);
  JxlEncoderSetFrameDistance(opts, distance);
  JxlEncoderFrameSettingsSetOption(opts, JXL_ENC_FRAME_SETTING_DECODING_SPEED,
                                   0);
  JxlEncoderFrameSettingsSetOption(opts, JXL_ENC_FRAME_SETTING_EFFORT, 6);
  JxlEncoderFrameSettingsSetOption(opts, JXL_ENC_FRAME_SETTING_BUFFERING, 2);

  if (JXL_ENC_SUCCESS !=
      JxlEncoderAddImageFrame(opts, &pixel_format,
                              static_cast<const void*>(data),
                              sizeof(uint8_t) * xsize * ysize * components)) {
    LOGW("encoder JxlEncoderAddImageFrame failed\n");
    return false;
  }

  JxlEncoderCloseInput(enc.get());

  compressed->resize(64);
  uint8_t* next_out = compressed->data();
  size_t avail_out = compressed->size() - (next_out - compressed->data());
  JxlEncoderStatus process_result = JXL_ENC_NEED_MORE_OUTPUT;
  while (process_result == JXL_ENC_NEED_MORE_OUTPUT) {
    process_result = JxlEncoderProcessOutput(enc.get(), &next_out, &avail_out);
    if (process_result == JXL_ENC_NEED_MORE_OUTPUT) {
      size_t offset = next_out - compressed->data();
      compressed->resize(compressed->size() * 2);
      next_out = compressed->data() + offset;
      avail_out = compressed->size() - offset;
    }
  }
  compressed->resize(next_out - compressed->data());
  if (JXL_ENC_SUCCESS != process_result) {
    LOGW("encoder JxlEncoderProcessOutput failed\n");
    return false;
  }

  return true;
};

#endif // IMAGEDECODER_ENCODER_JXL_H
