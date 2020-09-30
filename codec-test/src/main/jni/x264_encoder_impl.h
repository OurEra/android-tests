#ifndef x264encoder_h
#define x264encoder_h

#include "stdint.h"
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

class X264EncoderImpl
{
public:
  X264EncoderImpl();
  ~X264EncoderImpl();

  bool openX264Encoder();
  long x264EncoderProcess(x264_picture_t *pPicture, x264_nal_t **nals, int& nalsCount);
  bool closeX264Encoder();

  void setSourceFormat(unsigned int sourcformat);
  void setResolution(unsigned int w, unsigned int h);
  void setBitrate(unsigned int i_bitrate);
  void setFps(unsigned int fps);
  void setI_KeyInt_Max(unsigned int i_frame_max);
  void setQp_Max(unsigned int qp_max);
  void setQp_Min(unsigned int qp_min);

  void forceIDRFrame();

  void upgradeBitrateLevel();
  void declineBitrateLevel();
  void setLeastBitrateLevel();

private:

  x264_param_t *pParameter;
  x264_t *x264EncoderHandle;
  x264_picture_t *pOutput;

  unsigned int sourceFormat;
  unsigned int bitratelevel;
  unsigned int i_fps;
  unsigned int i_keyint_max;
  unsigned int width;
  unsigned int height;
  unsigned int qp_max;
  unsigned int qp_min;

  unsigned int current_f_rf_constant;
  unsigned int userSetting_f_rf_constant;

  int64_t frameNo;

  bool isForceIDRFrameEnabled;
};

#endif



/*
X264Encoder x264Encoder;

x264Encoder.setBitrate(512);
x264Encoder.setResolution(640,480);
x264Encoder.setFps(20);

FILE *inputFile = NULL;
FILE *outputFile = NULL;
inputFile = fopen("yuv_640x480.yuv","rb");
outputFile = fopen("h264_640x480.h264","wb");

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

fclose(inputFile);
fclose(outputFile);

x264_picture_clean(&inputPicture);

x264Encoder.closeX264Encoder();
*/
