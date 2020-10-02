#ifndef X264_ENCODER_IMPL_H
#define X264_ENCODER_IMPL_H

#include "encoder_base.h"
extern "C"
{
#include "x264.h"
}

enum bitrate_level
{
  HIGH_LEVEL = 0,
  STANDARD_LEVEL = 1,
  MEDIUM_LEVEL = 2,
  LOW_LEVEL = 3,
};

class X264EncoderImpl : public EncoderBase
{
public:
  X264EncoderImpl();
  ~X264EncoderImpl();

  // implements EncoderBase
  virtual int32_t InitEncode(CodecSetting& setting);
  virtual void    RegisterCallback(EncodeCallback * callback) { callback_ = callback; }
  virtual int32_t Encode(const uint8_t* frame, long long ts, bool force_key);
  virtual int32_t Release();

  void setBitrate(unsigned int i_bitrate);

  void upgradeBitrateLevel();
  void declineBitrateLevel();
  void setLeastBitrateLevel();

private:
  CodecSetting codecSetting_;
  x264_param_t *pParameter;
  x264_t *encoderHandler_;
  EncodeCallback *callback_;

  unsigned int sourceFormat;
  unsigned int bitratelevel;
  unsigned int current_f_rf_constant;
  unsigned int userSetting_f_rf_constant;
};

#endif
