#define LOGTAG "CODEC-h264-encoder"
#include "h264_encoder_impl.h"
#include <os_log.h>
#include <os_time.h>

#include "svc/codec_api.h"
#include "svc/codec_app_def.h"
#include "svc/codec_def.h"
#include "svc/codec_ver.h"

const bool kOpenH264EncoderDetailedLogging = false;

// QP scaling thresholds.
static const int kLowH264QpThreshold = 24;
static const int kHighH264QpThreshold = 37;

int CalcBufferSize(int width, int height) {

  size_t buffer_size = 0;
  int half_width = (width + 1) >> 1;
  int half_height = (height + 1) >> 1;
  return buffer_size = width * height + half_width * half_height * 2;
}


// Helper method used by H264EncoderImpl::Encode.
// Copies the encoded bytes from |info| to |encoded_image| and updates the
// fragmentation information of |frag_header|. The |encoded_image->_buffer| may
// be deleted and reallocated if a bigger buffer is required.
//
// After OpenH264 encoding, the encoded bytes are stored in |info| spread out
// over a number of layers and "NAL units". Each NAL unit is a fragment starting
// with the four-byte start code {0,0,0,1}. All of this data (including the
// start codes) is copied to the |encoded_image->_buffer| and the |frag_header|
// is updated to point to each fragment, with offsets and lengths set as to
// exclude the start codes.
static void RtpFragmentize(EncodedImage* encoded_image,
                           SFrameBSInfo* info) {
  // Calculate minimum buffer size required to hold encoded data.
  size_t required_size = 0;
  size_t fragments_count = 0;
  for (int layer = 0; layer < info->iLayerNum; ++layer) {
    const SLayerBSInfo& layerInfo = info->sLayerInfo[layer];
    for (int nal = 0; nal < layerInfo.iNalCount; ++nal, ++fragments_count) {
      // Ensure |required_size| will not overflow.
      required_size += layerInfo.pNalLengthInByte[nal];
    }
  }
  //logd("requsted size %d", required_size);

  // Iterate layers and NAL units, note each NAL unit as a fragment and copy
  // the data to |encoded_image->_buffer|.
  const uint8_t start_code[4] = {0, 0, 0, 1};
  size_t frag = 0;
  encoded_image->length = 0;
  for (int layer = 0; layer < info->iLayerNum; ++layer) {
    const SLayerBSInfo& layerInfo = info->sLayerInfo[layer];
    // Iterate NAL units making up this layer, noting fragments.
    size_t layer_len = 0;
    for (int nal = 0; nal < layerInfo.iNalCount; ++nal, ++frag) {
      // Because the sum of all layer lengths, |required_size|, fits in a
      // |size_t|, we know that any indices in-between will not overflow.
      layer_len += layerInfo.pNalLengthInByte[nal];
    }
    // Copy the entire layer's data (including start codes).
    memcpy(encoded_image->buffer + encoded_image->length,
           layerInfo.pBsBuf,
           layer_len);
    encoded_image->length += layer_len;
  }
}

H264EncoderImpl::H264EncoderImpl()
    : openh264_encoder_(NULL),
      width_(0),
      height_(0),
      max_frame_rate_(0.0f),
      target_bps_(0),
      max_bps_(0),
      frame_dropping_on_(false),
      key_frame_interval_(0),
      number_of_cores_(0) {
}

H264EncoderImpl::~H264EncoderImpl() {
  Release();
}

bool H264EncoderImpl::IsInitialized() const {
  return openh264_encoder_ != NULL;
}

int32_t H264EncoderImpl::InitEncode(CodecSetting& setting) {

  int32_t release_ret = Release();
  if (release_ret != 0) {
    return release_ret;
  }

  // Create encoder.
  if (WelsCreateSVCEncoder(&openh264_encoder_) != 0) {
    // Failed to create encoder.
    loge("Failed to create OpenH264 encoder");
    return -1;
  }
  if (kOpenH264EncoderDetailedLogging) {
    int trace_level = WELS_LOG_DETAIL;
    openh264_encoder_->SetOption(ENCODER_OPTION_TRACE_LEVEL,
                                 &trace_level);
  }
  // else WELS_LOG_DEFAULT is used by default.

  number_of_cores_ = 1;
  // Set internal settings from codec_settings
  width_ = setting.width;
  height_ = setting.height;
  max_frame_rate_ = setting.framerate;
  frame_dropping_on_ = 1;
  key_frame_interval_ = setting.keyinterval;

  max_bps_ = setting.bitrate;
  target_bps_ = setting.bitrate;

  SEncParamExt encoder_params = CreateEncoderParams();
  logd("period %d", encoder_params.uiIntraPeriod);

  // Initialize.
  if (openh264_encoder_->InitializeExt(&encoder_params) != 0) {
    loge("Failed to initialize OpenH264 encoder");
    Release();
    return -1;
  }
  // TODO(pbos): Base init params on these values before submitting.
  int video_format = EVideoFormatType::videoFormatI420;
  openh264_encoder_->SetOption(ENCODER_OPTION_DATAFORMAT,
                               &video_format);

  // Initialize encoded image. Default buffer size: size of unencoded data.
  encoded_image_.size = CalcBufferSize(width_, height_);
  encoded_image_.buffer = new uint8_t[encoded_image_.size];
  encoded_image_buffer_.reset(encoded_image_.buffer);
  encoded_image_.length = 0;

  SBitrateInfo target_bitrate;
  memset(&target_bitrate, 0, sizeof(SBitrateInfo));
  target_bitrate.iLayer = SPATIAL_LAYER_ALL,
  target_bitrate.iBitrate = target_bps_;
  openh264_encoder_->SetOption(ENCODER_OPTION_BITRATE,
                               &target_bitrate);
  openh264_encoder_->SetOption(ENCODER_OPTION_FRAME_RATE, &max_frame_rate_);
  logd("init encoder finish");

  return 0;
}

int32_t H264EncoderImpl::Release() {
  if (openh264_encoder_) {
    WelsDestroySVCEncoder(openh264_encoder_);
    openh264_encoder_ = nullptr;
  }
  encoded_image_.buffer = NULL;
  encoded_image_buffer_.reset();
  return 0;
}


int32_t H264EncoderImpl::Encode(const uint8_t* input_frame,
                                long long ts,
                                bool force_key) {
  if (!IsInitialized()) {
    return -1;
  }

  if (force_key) {
    // API doc says ForceIntraFrame(false) does nothing, but calling this
    // function forces a key frame regardless of the |bIDR| argument's value.
    // (If every frame is a key frame we get lag/delays.)
    logd("force key");
    openh264_encoder_->ForceIntraFrame(true);
  }
  // EncodeFrame input.
  SSourcePicture picture;
  memset(&picture, 0, sizeof(SSourcePicture));
  picture.iPicWidth = width_;
  picture.iPicHeight = height_;
  picture.iColorFormat = EVideoFormatType::videoFormatI420;
  picture.uiTimeStamp = ts;
  picture.iStride[0] = width_;
  picture.iStride[1] = width_ / 2;
  picture.iStride[2] = width_ / 2;
  picture.pData[0] = const_cast<uint8_t*>(input_frame);
  picture.pData[1] = const_cast<uint8_t*>(input_frame + width_ * height_);
  picture.pData[2] = const_cast<uint8_t*>(picture.pData[1] + width_* height_/ 4);

  // EncodeFrame output.
  SFrameBSInfo info;
  memset(&info, 0, sizeof(SFrameBSInfo));

  // Encode!
  int64_t b_ts = os_get_monitictime();
  int enc_ret = openh264_encoder_->EncodeFrame(&picture, &info);
  int64_t now_ts = os_get_monitictime();
  if (enc_ret != 0) {
    loge("OpenH264 frame encoding failed, EncodeFrame returned %d", enc_ret);
    return -1;
  }

  // Split encoded image up into fragments. This also updates |encoded_image_|.
  RtpFragmentize(&encoded_image_, &info);

  // Encoder can skip frames to save bandwidth in which case
  // |encoded_image_.length| == 0.
  if (encoded_image_.length > 0) {
    logd("timediff : %lld(ms) length %dk type %d", (now_ts - b_ts) / 1000000, encoded_image_.length / 1024, info.eFrameType);
    // Deliver encoded image.
    if (callback_) {
      callback_->onEncoded(encoded_image_);
    }
  }
  return 0;
}

// Initialization parameters.
// There are two ways to initialize. There is SEncParamBase (cleared with
// memset(&p, 0, sizeof(SEncParamBase)) used in Initialize, and SEncParamExt
// which is a superset of SEncParamBase (cleared with GetDefaultParams) used
// in InitializeExt.
SEncParamExt H264EncoderImpl::CreateEncoderParams() const {
  SEncParamExt encoder_params;
  openh264_encoder_->GetDefaultParams(&encoder_params);
  encoder_params.iUsageType = CAMERA_VIDEO_REAL_TIME;
  //encoder_params.iUsageType = SCREEN_CONTENT_REAL_TIME;

  encoder_params.iPicWidth = width_;
  encoder_params.iPicHeight = height_;
  encoder_params.iTargetBitrate = target_bps_;
  encoder_params.iMaxBitrate = max_bps_;
  // Rate Control mode
  encoder_params.iRCMode = RC_OFF_MODE;
  //encoder_params.iRCMode = RC_BITRATE_MODE;

  // The following parameters are extension parameters (they're in SEncParamExt,
  // not in SEncParamBase).
  encoder_params.bEnableFrameSkip = frame_dropping_on_;
  // |uiIntraPeriod|    - multiple of GOP size
  // |keyFrameInterval| - number of frames
  encoder_params.uiIntraPeriod = key_frame_interval_;
  encoder_params.uiMaxNalSize = 0;
  logd("interval %d", encoder_params.uiIntraPeriod);
  // Threading model: use auto.
  //  0: auto (dynamic imp. internal encoder)
  //  1: single thread (default value)
  // >1: number of threads
  encoder_params.iMultipleThreadIdc = 1;
  // The base spatial layer 0 is the only one we use.
  encoder_params.sSpatialLayers[0].iVideoWidth = encoder_params.iPicWidth;
  encoder_params.sSpatialLayers[0].iVideoHeight = encoder_params.iPicHeight;
  encoder_params.sSpatialLayers[0].fFrameRate = encoder_params.fMaxFrameRate;
  encoder_params.sSpatialLayers[0].iSpatialBitrate =
      encoder_params.iTargetBitrate;
  encoder_params.sSpatialLayers[0].iMaxSpatialBitrate =
      encoder_params.iMaxBitrate;

  // case H264PacketizationMode::NonInterleaved:
  // When uiSliceMode = SM_FIXEDSLCNUM_SLICE, uiSliceNum = 0 means auto
  // design it with cpu core number.
  // TODO(sprang): Set to 0 when we understand why the rate controller borks
  //               when uiSliceNum > 1.
  encoder_params.sSpatialLayers[0].sSliceArgument.uiSliceNum = 1;
  encoder_params.sSpatialLayers[0].sSliceArgument.uiSliceMode =
      SM_FIXEDSLCNUM_SLICE;
  encoder_params.iMaxQp = 36;
  encoder_params.iMinQp = 10;
  return encoder_params;
}
