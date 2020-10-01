#ifndef CODEC_COMMON_H_
#define CODEC_COMMON_H_

#include <stdint.h>

struct CodecSetting {
  size_t width;
  size_t height;
  // bps
  size_t bitrate;
  size_t framerate;
  size_t keyinterval;
};

struct EncodedImage {
  uint8_t* buffer;
  size_t length;
  size_t size;
};

struct EncodedStats {
  int32_t max_ms;
  int32_t min_ms;
  int32_t aver_ms;
};


class EncodeCallback {
public:
  virtual void onEncoded(EncodedImage& image) = 0;
};

#endif  // CODEC_COMMON_H_





