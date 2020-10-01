#ifndef H264_BASE_H_
#define H264_BASE_H_

#include "codec_common.h"
class EncoderBase {

public:
  virtual int32_t InitEncode(CodecSetting& setting) = 0;
  virtual void    RegisterCallback(EncodeCallback * callback) = 0;
  virtual int32_t Encode(const uint8_t* frame, long long ts, bool force_key) = 0;
  virtual int32_t Release() = 0;
};

#endif
