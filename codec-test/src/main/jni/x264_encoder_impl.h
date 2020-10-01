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

  int64_t frameNo;
};

#endif



/*
X264Encoder x264Encoder;

x264_picture_t inputPicture;
x264_picture_alloc(&inputPicture, X264_CSP_I420, 640, 480);

x264_nal_t *p_nals = NULL;
int nalsCount = 0;

if(x264Encoder.openX264Encoder())
{
    for(int j=0; j<20; j++)
    {
        fread(inputPicture.img.plane[0],1,640*480, inputFile);
        fread(inputPicture.img.plane[1],1,640*480/4, inputFile);
        fread(inputPicture.img.plane[2],1,640*480/4, inputFile);

        x264Encoder.x264EncoderProcess(&inputPicture,&p_nals,nalsCount);

        if(p_nals)
        {
            for(int i=0; i<nalsCount; i++)
            {
                fwrite(p_nals[i].p_payload, p_nals[i].i_payload, 1, outputFile);
            }
        }
    }

}

x264_picture_clean(&inputPicture);

x264Encoder.closeX264Encoder();
*/
