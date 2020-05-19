#ifndef H264_ENCODER_IMPL_H_
#define H264_ENCODER_IMPL_H_

#include "svc/codec_app_def.h"

class ISVCEncoder;

namespace qiniutest {

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

  // The result of encoding - an EncodedImage and RTPFragmentationHeader - are
  // passed to the encode complete callback.
  int32_t Encode(const VideoFrame& frame, bool force_key);

 private:
  bool IsInitialized() const;
  SEncParamExt CreateEncoderParams() const;

  webrtc::H264BitstreamParser h264_bitstream_parser_;

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
