#include <stdlib.h>
#include <string.h>
#include "x264_encoder_impl.h"

X264EncoderImpl::X264EncoderImpl() {
  bitratelevel = STANDARD_LEVEL;
  qp_max = 30;
  qp_min = 0;
  i_fps = 20;
  i_keyint_max = 300;
  width = 352;
  height = 288;

  frameNo = 0;
  isForceIDRFrameEnabled = false;

  pParameter = NULL;
  X264EncoderImplHandle = NULL;
  //    pPicture = NULL;
  pOutput = NULL;
}

X264EncoderImpl::~X264EncoderImpl() {
  closeX264EncoderImpl();
}

void X264EncoderImpl::setSourceFormat(unsigned int sourcformat) {
  sourceFormat = sourcformat;
}

void X264EncoderImpl::setResolution(unsigned int w, unsigned int h) {
  width = w;
  height = h;
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

void X264EncoderImpl::setFps(unsigned int fps) {
  i_fps = fps;
}

void X264EncoderImpl::setI_KeyInt_Max(unsigned int i_frame_max) {
  i_keyint_max = i_frame_max;
}

void X264EncoderImpl::setQp_Max(unsigned int qp_max) {
  qp_max = qp_max;
}

void X264EncoderImpl::setQp_Min(unsigned int qp_min) {
  qp_min = qp_min;
}

bool X264EncoderImpl::openX264EncoderImpl() {
  closeX264EncoderImpl();

  if(!pParameter)
  {
      pParameter = (x264_param_t *)malloc(sizeof(x264_param_t));

      if (!pParameter) {
          this->closeX264EncoderImpl();

          return false;
      }

      memset(pParameter, 0, sizeof(x264_param_t));
  }

  int ret = x264_param_default_preset(pParameter, "ultrafast", "zerolatency");
  if (ret != 0) {

      this->closeX264EncoderImpl();

      return false;
  }

  pParameter->i_level_idc = 30;

  pParameter->i_width = width;
  pParameter->i_height = height;

  pParameter->b_deterministic = 1;
//  pParameter->b_sliced_threads = 1;
  pParameter->i_threads = 1;

  pParameter->i_csp = X264_CSP_I420;//X264_CSP_NV12;//X264_CSP_I420;

  pParameter->i_fps_num = i_fps;
  pParameter->i_fps_den = 1;
  pParameter->i_bframe = 0;
  pParameter->i_keyint_max = i_keyint_max;

//  pParameter->b_open_gop = 1;

//  pParameter->rc.i_bitrate = i_bitrate;

  pParameter->rc.i_rc_method = X264_RC_CRF;//X264_RC_CQP;

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

  // from huxiaopeng
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
  //  pParameter->analyse.i_noise_reduction = 1;

  pParameter->i_slice_max_size = 1200;

//  pParameter->i_nal_hrd = X264_NAL_HRD_NONE;

  pParameter->b_deblocking_filter = 1;
  pParameter->i_deblocking_filter_alphac0 = 4;
  pParameter->i_deblocking_filter_beta = 4;

  pParameter->rc.b_mb_tree = 0;

  pParameter->i_log_level = X264_LOG_NONE;

  if(x264_param_apply_profile(pParameter, "baseline")) {
    closeX264EncoderImpl();
    return false;
  }

  if (!X264EncoderImplHandle) {
    X264EncoderImplHandle = x264_encoder_open(pParameter);

    if (!X264EncoderImplHandle) {
      closeX264EncoderImpl();
      return false;
    }
  }

  /*
  if (!pPicture) {
  pPicture = (x264_picture_t *)malloc(sizeof(x264_picture_t));

  if (!pPicture) {

  this->closeX264EncoderImpl();

  return false;
  }

  memset(pPicture, 0, sizeof(x264_picture_t));
  }

  if (x264_picture_alloc(pPicture, X264_CSP_I420, width, height)) {

  this->closeX264EncoderImpl();

  return false;
  }
  */

  if (!pOutput) {
    pOutput = (x264_picture_t *)malloc(sizeof(x264_picture_t));
    if (!pOutput) {
      this->closeX264EncoderImpl();
      return false;
    }
    memset(pOutput, 0, sizeof(x264_picture_t));
  }
  return true;
}

void X264EncoderImpl::forceIDRFrame() {
  isForceIDRFrameEnabled = true;
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

  x264_encoder_reconfig(X264EncoderImplHandle, pParameter);
}

void X264EncoderImpl::setLeastBitrateLevel() {
    pParameter->rc.f_rf_constant = 32;
    current_f_rf_constant = pParameter->rc.f_rf_constant;

    x264_encoder_reconfig(X264EncoderImplHandle, pParameter);
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

  x264_encoder_reconfig(X264EncoderImplHandle, pParameter);
}

long X264EncoderImpl::X264EncoderImplProcess(x264_picture_t *pPicture, x264_nal_t **nals, int& nalsCount) {
  pPicture->i_pts = (int64_t)(frameNo * pParameter->i_fps_den);
  pPicture->i_type = X264_TYPE_AUTO;
  pPicture->i_qpplus1 = 0;//X264_QP_AUTO;

  if (isForceIDRFrameEnabled) {
      pPicture->i_type = X264_TYPE_IDR;
      isForceIDRFrameEnabled = false;
  }

  int32_t framesize = -1;


  framesize = x264_encoder_encode(X264EncoderImplHandle, nals, &nalsCount, pPicture, pOutput);
  if (framesize>0) {
      frameNo++;
  }

  return framesize;
}

/*
 long X264EncoderImpl::X264EncoderImplProcess(uint8_t *pSrcData, int srcDataSize, x264_nal_t **nals, int& nalsCount) {

 pPicture->img.plane[0] = pSrcData;
 pPicture->img.plane[1] = pSrcData + srcDataSize*2/3;
 pPicture->img.plane[2] = pSrcData + srcDataSize*2/3 + srcDataSize/6;
 pPicture->i_pts = (int64_t)(frameNo * pParameter->i_fps_den);
 pPicture->i_type = X264_TYPE_AUTO;
 pPicture->i_qpplus1 = X264_QP_AUTO;

 if (isForceIDRFrameEnabled) {
 pPicture->i_type = X264_TYPE_IDR;
 isForceIDRFrameEnabled = false;
 }

 int32_t framesize = -1;


 framesize = x264_encoder_encode(X264EncoderImplHandle, nals, &nalsCount, pPicture, pOutput);

 if (framesize>0) {
 frameNo++;
 }

 return framesize;
 }
 */

bool X264EncoderImpl::closeX264EncoderImpl() {
  if (pOutput) {
      free(pOutput);
      pOutput = NULL;
  }
  /*
  if (pPicture) {
  free(pPicture);
  pPicture = NULL;
  }
  */
  if (pParameter) {
      free(pParameter);
      pParameter = NULL;
  }

  if (X264EncoderImplHandle) {
      x264_encoder_close(X264EncoderImplHandle);
      X264EncoderImplHandle = NULL;
  }
  return true;
}
