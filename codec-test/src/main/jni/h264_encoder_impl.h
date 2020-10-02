#ifndef H264_ENCODER_IMPL_H_
#define H264_ENCODER_IMPL_H_

#include "svc/codec_app_def.h"
#include "encoder_base.h"
#include <memory>

class ISVCEncoder;
class H264EncoderImpl : public EncoderBase {

 public:
  explicit H264EncoderImpl();
  ~H264EncoderImpl();

  // implements EncoderBase
  virtual int32_t InitEncode(CodecSetting& setting);
  virtual void    RegisterCallback(EncodeCallback * callback) { callback_ = callback; }
  virtual int32_t Encode(const uint8_t* frame, long long ts, bool force_key);
  virtual int32_t Release();

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

  EncodeCallback *callback_;
  EncodedImage encoded_image_;
  EncodedStats stats_;
  std::unique_ptr<uint8_t[]> encoded_image_buffer_;
};

#endif  // H264_ENCODER_IMPL_H_
