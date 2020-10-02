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


//redefinition enums in x264 and openh264
// cut from codec_def.h and x264.h
enum nal_unit_type_e
{
    NAL_UNKNOWN     = 0,
    NAL_SLICE       = 1,
    NAL_SLICE_DPA   = 2,
    NAL_SLICE_DPB   = 3,
    NAL_SLICE_DPC   = 4,
    NAL_SLICE_IDR   = 5,    /* ref_idc != 0 */
    NAL_SEI         = 6,    /* ref_idc == 0 */
    NAL_SPS         = 7,
    NAL_PPS         = 8,
    NAL_AUD         = 9,
    NAL_FILLER      = 12,
    /* ref_idc == 0 for 6,9,10,11,12 */
};
enum nal_priority_e
{
    NAL_PRIORITY_DISPOSABLE = 0,
    NAL_PRIORITY_LOW        = 1,
    NAL_PRIORITY_HIGH       = 2,
    NAL_PRIORITY_HIGHEST    = 3,
};


#endif  // CODEC_COMMON_H_





