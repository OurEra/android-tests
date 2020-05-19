#ifndef H264_ENCODER_IMPL_H_
#define H264_ENCODER_IMPL_H_

#include "svc/codec_app_def.h"
#include <stdint.h>
#include <memory>

class ISVCEncoder;

namespace qiniutest {

struct EncodedImage {
  uint8_t* _buffer;
  size_t _length;
  size_t _size;
};

class H264EncoderImpl {

 public:
  explicit H264EncoderImpl();
  ~H264EncoderImpl();

  // |max_payload_size| is ignored.
  // The following members of |codec_settings| are used. The rest are ignored.
  // - codecType (must be kVideoCodecH264)
  // - targetBitrate
  // - maxFramerate
  // - width
  // - height
  int32_t InitEncode();
  int32_t Release();

  int32_t Encode(const uint8_t* frame, long long ts, bool force_key);

 private:
  bool IsInitialized() const;
  SEncParamExt CreateEncoderParams() const;

  ISVCEncoder* openh264_encoder_;
  // Settings that are used by this encoder.
  int width_;
  int height_;
  float max_frame_rate_;
  uint32_t target_bps_;
  uint32_t max_bps_;
  // H.264 specifc parameters
  bool frame_dropping_on_;
  int key_frame_interval_;

  int32_t number_of_cores_;

  EncodedImage encoded_image_;
  std::unique_ptr<uint8_t[]> encoded_image_buffer_;
};

}  // namespace qiniutest

#endif  // H264_ENCODER_IMPL_H_
