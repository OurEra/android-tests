#define LOGTAG "CODEC-x264-codec"

#include <stdlib.h>
#include <string.h>
#include "x264_encoder_impl.h"
#include <os_log.h>

void x264_log_redirect( void *p_unused, int i_level, const char *psz_fmt, va_list arg) {
  log_info(LOGTAG, psz_fmt, arg);
}

X264EncoderImpl::X264EncoderImpl() {
  bitratelevel = STANDARD_LEVEL;
  pParameter = NULL;
  encoderHandler_ = NULL;
}

X264EncoderImpl::~X264EncoderImpl() {
  Release();
}

void X264EncoderImpl::setBitrate(unsigned int i_bitrate) {
  if (i_bitrate > 0 && i_bitrate <= 64) {
    bitratelevel = LOW_LEVEL;
  } else if(i_bitrate > 64 && i_bitrate <= 128) {
    bitratelevel = MEDIUM_LEVEL;
  } else if (i_bitrate > 128 && i_bitrate <= 256) {
    bitratelevel = STANDARD_LEVEL;
  } else if (i_bitrate > 256 && i_bitrate <= 384) {
    bitratelevel = HIGH_LEVEL;
  } else if (i_bitrate > 384 && i_bitrate <= 512) {
    bitratelevel = HIGH_LEVEL;
  } else {
    bitratelevel = STANDARD_LEVEL;
  }
}

int32_t X264EncoderImpl::InitEncode(CodecSetting& setting) {
  Release();

  if(!pParameter) {
    pParameter = (x264_param_t *)malloc(sizeof(x264_param_t));
    if (!pParameter) {
      Release();
      return false;
    }
    memset(pParameter, 0, sizeof(x264_param_t));
  }

  int ret = x264_param_default_preset(pParameter, "ultrafast", "zerolatency");
  if (ret != 0) {
    Release();
    return false;
  }

  codecSetting_ = setting;

  pParameter->i_level_idc = 30;

  pParameter->i_width = codecSetting_.width;
  pParameter->i_height = codecSetting_.height;

  pParameter->b_deterministic = 1;
  //pParameter->b_sliced_threads = 1;
  //pParameter->i_threads = 1;

  pParameter->i_csp = X264_CSP_I420;

  pParameter->i_fps_num = codecSetting_.framerate;
  pParameter->i_fps_den = 1;
  pParameter->i_bframe = 0;
  pParameter->i_keyint_max = codecSetting_.keyinterval;

  //pParameter->b_open_gop = 1;
  //pParameter->rc.i_bitrate = i_bitrate;
  pParameter->rc.i_rc_method = X264_RC_CRF;

  if (this->bitratelevel == LOW_LEVEL) {
    pParameter->rc.f_rf_constant = 32;
  } else if(this->bitratelevel == MEDIUM_LEVEL) {
    pParameter->rc.f_rf_constant = 29;
  } else if (this->bitratelevel == STANDARD_LEVEL) {
    pParameter->rc.f_rf_constant = 26;
  } else if (this->bitratelevel == HIGH_LEVEL) {
    pParameter->rc.f_rf_constant = 24;
  } else {
   pParameter->rc.f_rf_constant = 24;
  }

  current_f_rf_constant = pParameter->rc.f_rf_constant;
  userSetting_f_rf_constant = pParameter->rc.f_rf_constant;

  pParameter->analyse.b_transform_8x8 = 1;
  pParameter->rc.f_aq_strength = 1.5;

  pParameter->rc.i_aq_mode = 0;
  pParameter->rc.f_qcompress = 0.0;
  pParameter->rc.f_ip_factor = 0.5;
  pParameter->rc.f_rate_tolerance = 0.1;

  pParameter->analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO;
  pParameter->analyse.i_me_method = X264_ME_DIA;
  pParameter->analyse.i_me_range = 16;
  pParameter->analyse.i_subpel_refine = 2;
  //pParameter->analyse.i_noise_reduction = 1;

  pParameter->i_slice_max_size = 1200;

  //pParameter->i_nal_hrd = X264_NAL_HRD_NONE;

  pParameter->b_deblocking_filter = 1;
  pParameter->i_deblocking_filter_alphac0 = 4;
  pParameter->i_deblocking_filter_beta = 4;

  pParameter->rc.b_mb_tree = 0;
  pParameter->i_log_level = X264_LOG_INFO;

  if(x264_param_apply_profile(pParameter, "baseline")) {
    Release();
    return false;
  }
  pParameter->pf_log = x264_log_redirect;

  if (!encoderHandler_) {
    encoderHandler_ = x264_encoder_open(pParameter);
    if (!encoderHandler_) {
      Release();
      return -1;
    }
  }
  logi("open x264 handler %p", encoderHandler_);
  return 0;
}

void X264EncoderImpl::upgradeBitrateLevel() {
  /*
  if (this->bitratelevel == HIGH_LEVEL) {
      return;
  }

  this->bitratelevel++;

  if (this->bitratelevel == LOW_LEVEL) {
      pParameter->rc.f_rf_constant = 30;
  }else if(this->bitratelevel == MEDIUM_LEVEL){
      pParameter->rc.f_rf_constant = 27;
  }else if (this->bitratelevel == STANDARD_LEVEL) {
      pParameter->rc.f_rf_constant = 24;
  }else if (this->bitratelevel == HIGH_LEVEL) {
      pParameter->rc.f_rf_constant = 22;
  }else {
      pParameter->rc.f_rf_constant = 23;
  }
  */

  if (userSetting_f_rf_constant >= current_f_rf_constant) {
      return;
  }

  pParameter->rc.f_rf_constant--;
  current_f_rf_constant = pParameter->rc.f_rf_constant;

  x264_encoder_reconfig(encoderHandler_, pParameter);
}

void X264EncoderImpl::setLeastBitrateLevel() {
    pParameter->rc.f_rf_constant = 32;
    current_f_rf_constant = pParameter->rc.f_rf_constant;

    x264_encoder_reconfig(encoderHandler_, pParameter);
}

void X264EncoderImpl::declineBitrateLevel() {
  /*
  if (this->bitratelevel == LOW_LEVEL) {
      return;
  }

  this->bitratelevel--;

  if (this->bitratelevel == LOW_LEVEL) {
      pParameter->rc.f_rf_constant = 30;
  }else if(this->bitratelevel == MEDIUM_LEVEL){
      pParameter->rc.f_rf_constant = 27;
  }else if (this->bitratelevel == STANDARD_LEVEL) {
      pParameter->rc.f_rf_constant = 24;
  }else if (this->bitratelevel == HIGH_LEVEL) {
      pParameter->rc.f_rf_constant = 22;
  }else {
      pParameter->rc.f_rf_constant = 23;
  }
  */

  if (32 <= current_f_rf_constant) {
      return;
  }

  pParameter->rc.f_rf_constant++;
  current_f_rf_constant = pParameter->rc.f_rf_constant;

  x264_encoder_reconfig(encoderHandler_, pParameter);
}

int32_t X264EncoderImpl::Encode(const uint8_t* frame, long long ts, bool force_key) {
  x264_picture_t inPicture, outPicture;
  inPicture = {0};
  outPicture = {0};
  inPicture.img.i_csp = X264_CSP_I420;
  inPicture.img.i_plane = 3;
  inPicture.img.i_stride[0] = codecSetting_.width;
  inPicture.img.i_stride[1] = codecSetting_.width / 2;
  inPicture.img.i_stride[2] = codecSetting_.width / 2;
  inPicture.img.plane[0] = const_cast<uint8_t*>(frame);
  inPicture.img.plane[1] = const_cast<uint8_t*>(frame + codecSetting_.width * codecSetting_.height);
  inPicture.img.plane[2] = const_cast<uint8_t*>(inPicture.img.plane[1] + codecSetting_.width * codecSetting_.height / 4);;
  inPicture.i_pts = ts;
  inPicture.i_type = X264_TYPE_AUTO;
  inPicture.i_qpplus1 = X264_QP_AUTO;
  if (force_key) {
    inPicture.i_type = X264_TYPE_IDR;
  }

  x264_nal_t *nals = NULL;
  int nalsCount = 0;
  int32_t framesize = -1;
  framesize = x264_encoder_encode(encoderHandler_, &nals, &nalsCount, &inPicture, &outPicture);
  // Deliver encoded image.
  if (callback_) {
    EncodedImage image;
    image.buffer = nals[0].p_payload;
    image.length = framesize;
    image.size = framesize;
    callback_->onEncoded(image);
  }

  logi("x264 encoded size %d nalu %d", framesize, nalsCount);
  return framesize;
}

int32_t X264EncoderImpl::Release() {
  if (pParameter) {
      free(pParameter);
      pParameter = NULL;
  }

  if (encoderHandler_) {
      x264_encoder_close(encoderHandler_);
      encoderHandler_ = NULL;
  }
  return 0;
}
