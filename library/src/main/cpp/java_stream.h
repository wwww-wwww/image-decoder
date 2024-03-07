//
// Created by len on 23/12/20.
//

#ifndef IMAGEDECODER_JAVA_STREAM_H
#define IMAGEDECODER_JAVA_STREAM_H

#include "log.h"
#include "stream.h"
#include <jni.h>
#include <memory>
#include <stdlib.h>

void init_java_stream(JNIEnv* env);

std::shared_ptr<Stream> read_all_java_stream(JNIEnv* env, jobject jstream);

bool write_all_java_stream(JNIEnv* env, jobject jstream, const uint8_t* bytes,
                           size_t size);

#endif // IMAGEDECODER_JAVA_STREAM_H
