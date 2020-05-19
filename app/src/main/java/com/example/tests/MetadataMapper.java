package com.example.tests;

import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.util.Log;
import android.util.Range;
import android.util.Rational;

import static android.hardware.camera2.CameraCharacteristics.CONTROL_AE_AVAILABLE_MODES;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AE_COMPENSATION_RANGE;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AE_COMPENSATION_STEP;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AF_AVAILABLE_MODES;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AF_MODE_AUTO;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AF_MODE_CONTINUOUS_PICTURE;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AF_MODE_CONTINUOUS_VIDEO;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AF_MODE_EDOF;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AF_MODE_MACRO;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AF_MODE_OFF;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AVAILABLE_SCENE_MODES;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AWB_AVAILABLE_MODES;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AWB_MODE_AUTO;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AWB_MODE_CLOUDY_DAYLIGHT;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AWB_MODE_DAYLIGHT;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AWB_MODE_FLUORESCENT;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AWB_MODE_INCANDESCENT;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AWB_MODE_SHADE;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AWB_MODE_TWILIGHT;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_AWB_MODE_WARM_FLUORESCENT;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_MAX_REGIONS_AE;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_MAX_REGIONS_AF;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_SCENE_MODE_ACTION;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_SCENE_MODE_BARCODE;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_SCENE_MODE_BEACH;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_SCENE_MODE_CANDLELIGHT;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_SCENE_MODE_DISABLED;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_SCENE_MODE_FIREWORKS;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_SCENE_MODE_LANDSCAPE;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_SCENE_MODE_NIGHT;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_SCENE_MODE_PARTY;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_SCENE_MODE_PORTRAIT;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_SCENE_MODE_SNOW;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_SCENE_MODE_SPORTS;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_SCENE_MODE_STEADYPHOTO;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_SCENE_MODE_SUNSET;
import static android.hardware.camera2.CameraCharacteristics.CONTROL_SCENE_MODE_THEATRE;
import static android.hardware.camera2.CameraCharacteristics.FLASH_INFO_AVAILABLE;
import static android.hardware.camera2.CameraCharacteristics.SCALER_AVAILABLE_MAX_DIGITAL_ZOOM;
import static android.hardware.camera2.CameraCharacteristics.STATISTICS_INFO_MAX_FACE_COUNT;
import static android.hardware.camera2.CameraMetadata.FLASH_MODE_OFF;
import static android.hardware.camera2.CameraMetadata.FLASH_MODE_SINGLE;
import static android.hardware.camera2.CameraMetadata.FLASH_MODE_TORCH;

//import static android.hardware.camera2.CameraCharacteristics.CONTROL_SCENE_MODE_HDR;

public class MetadataMapper {
    private static String TAG = "MetadataMapper";

    public static CameraParameter createParameter(CameraCharacteristics p, CaptureRequest.Builder request) {

        CameraParameter param = new CameraParameter();
        buildSceneModes(p, param);
        buildFlashModes(p, param);
        param.set(CameraParameter.KEY_FLASH_MODE, flashModeFromMeta(request.get(CaptureRequest.FLASH_MODE)));
        buildFocusModes(p, param);
        param.set(CameraParameter.KEY_FOCUS_MODE, flashModeFromMeta(request.get(CaptureRequest.CONTROL_AF_MODE)));
        buildWhiteBalances(p, param);
        param.set(CameraParameter.KEY_WHITE_BALANCE, flashModeFromMeta(request.get(CaptureRequest.CONTROL_AWB_MODE)));

        // TODO: Populate mSupportedFeatures

        Range<Integer> ecRange = p.get(CONTROL_AE_COMPENSATION_RANGE);
        param.set(CameraParameter.KEY_MIN_EXPOSURE_COMPENSATION, ecRange.getLower());
        param.set(CameraParameter.KEY_MAX_EXPOSURE_COMPENSATION, ecRange.getUpper());
        Rational ecStep = p.get(CONTROL_AE_COMPENSATION_STEP);
        param.set(CameraParameter.KEY_EXPOSURE_COMPENSATION_STEP, Float.toString(ecStep.getNumerator() / ecStep.getDenominator()));
        param.set(CameraParameter.KEY_EXPOSURE_COMPENSATION, request.get(CaptureRequest.CONTROL_AE_EXPOSURE_COMPENSATION));

        param.set(CameraParameter.KEY_MAX_NUM_METERING_AREAS, p.get(CONTROL_MAX_REGIONS_AE));
        param.set(CameraParameter.KEY_MAX_NUM_FOCUS_AREAS, p.get(CONTROL_MAX_REGIONS_AF));

        int von = request.get(CaptureRequest.CONTROL_VIDEO_STABILIZATION_MODE);
        param.set(CameraParameter.KEY_VIDEO_STABILIZATION, von == 1 ? "true" : "false");

        // TODO: Populate mHorizontalViewAngle
        // TODO: Populate mVerticalViewAngle
        // TODO: Populate mZoomRatioList
        // TODO: Populate mMaxZoomIndex
        // TODO: Detect other features

        return param;
    }

    private static void buildSceneModes(CameraCharacteristics character, CameraParameter para) {
        int[] scenes = character.get(CONTROL_AVAILABLE_SCENE_MODES);
        if (scenes != null) {
            StringBuilder builder = new StringBuilder();
            for (int scene : scenes) {
                String equiv = sceneModeFromMeta(scene);
                if (equiv != null) {
                    builder.append(equiv);
                    builder.append(",");
                }
            }
            builder.deleteCharAt(builder.length() - 1);
            para.set(CameraParameter.KEY_SUPPORTED_SCENE_MODES,
                    builder.toString());
        }
    }

    private static void buildFlashModes(CameraCharacteristics character, CameraParameter para) {

        StringBuilder builder = new StringBuilder();
        builder.append(CameraParameter.FLASH_MODE_OFF);
        builder.append(",");
        if (character.get(FLASH_INFO_AVAILABLE)) {
            builder.append(CameraParameter.FLASH_MODE_AUTO);
            builder.append(",");
            builder.append(CameraParameter.FLASH_MODE_ON);
            builder.append(",");
            builder.append(CameraParameter.FLASH_MODE_TORCH);
            builder.append(",");
            for (int expose : character.get(CONTROL_AE_AVAILABLE_MODES)) {
                if (expose == CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE) {
                    builder.append(CameraParameter.FLASH_MODE_RED_EYE);
                    builder.append(",");
                }
            }
        }
        builder.deleteCharAt(builder.length() - 1);
        para.set(CameraParameter.KEY_SUPPORTED_FLASH_MODES,
                builder.toString());
    }

    private static void buildFocusModes(CameraCharacteristics character, CameraParameter para) {
        int[] focuses = character.get(CONTROL_AF_AVAILABLE_MODES);
        if (focuses != null) {
            StringBuilder builder = new StringBuilder();
            for (int focus : focuses) {
                String equiv = focusModeFromMeta(focus);
                if (equiv != null) {
                    builder.append(equiv);
                    builder.append(",");
                }
            }
            builder.deleteCharAt(builder.length() - 1);
            para.set(CameraParameter.KEY_SUPPORTED_FOCUS_MODES,
                    builder.toString());
        }
    }

    private static void buildWhiteBalances(CameraCharacteristics character, CameraParameter para) {
        int[] bals = character.get(CONTROL_AWB_AVAILABLE_MODES);
        if (bals != null) {
            StringBuilder builder = new StringBuilder();
            for (int bal : bals) {
                String equiv = whiteBalanceFromMeta(bal);
                if (equiv != null) {
                    builder.append(equiv);
                    builder.append(",");
                }
            }
            builder.deleteCharAt(builder.length() - 1);
            para.set(CameraParameter.KEY_SUPPORTED_WHITE_BALANCE ,
                    builder.toString());
        }
    }

    public static String focusModeFromMeta(int fm) {
        switch (fm) {
            case CONTROL_AF_MODE_AUTO:
                return CameraParameter.FOCUS_MODE_AUTO;
            case CONTROL_AF_MODE_CONTINUOUS_PICTURE:
                return CameraParameter.FOCUS_MODE_CONTINUOUS_PICTURE;
            case CONTROL_AF_MODE_CONTINUOUS_VIDEO:
                return CameraParameter.FOCUS_MODE_CONTINUOUS_VIDEO;
            case CONTROL_AF_MODE_EDOF:
                return CameraParameter.FOCUS_MODE_EDOF;
            case CONTROL_AF_MODE_OFF:
                return CameraParameter.FOCUS_MODE_FIXED;
            // TODO: We cannot support INFINITY
            case CONTROL_AF_MODE_MACRO:
                return CameraParameter.FOCUS_MODE_MACRO;
        }
        Log.w(TAG, "Unable to convert from API 2 focus mode: " + fm);
        return null;
    }

    public static String sceneModeFromMeta(int sm) {
        switch (sm) {
            case CONTROL_SCENE_MODE_DISABLED:
                return CameraParameter.SCENE_MODE_AUTO;
            case CONTROL_SCENE_MODE_ACTION:
                return CameraParameter.SCENE_MODE_ACTION;
            case CONTROL_SCENE_MODE_BARCODE:
                return CameraParameter.SCENE_MODE_BARCODE;
            case CONTROL_SCENE_MODE_BEACH:
                return CameraParameter.SCENE_MODE_BEACH;
            case CONTROL_SCENE_MODE_CANDLELIGHT:
                return CameraParameter.SCENE_MODE_CANDLELIGHT;
            case CONTROL_SCENE_MODE_FIREWORKS:
                return CameraParameter.SCENE_MODE_FIREWORKS;
            case CONTROL_SCENE_MODE_LANDSCAPE:
                return CameraParameter.SCENE_MODE_LANDSCAPE;
            case CONTROL_SCENE_MODE_NIGHT:
                return CameraParameter.SCENE_MODE_NIGHT;
            case CONTROL_SCENE_MODE_PARTY:
                return CameraParameter.SCENE_MODE_PARTY;
            case CONTROL_SCENE_MODE_PORTRAIT:
                return CameraParameter.SCENE_MODE_PORTRAIT;
            case CONTROL_SCENE_MODE_SNOW:
                return CameraParameter.SCENE_MODE_SNOW;
            case CONTROL_SCENE_MODE_SPORTS:
                return CameraParameter.SCENE_MODE_SPORTS;
            case CONTROL_SCENE_MODE_STEADYPHOTO:
                return CameraParameter.SCENE_MODE_STEADYPHOTO;
            case CONTROL_SCENE_MODE_SUNSET:
                return CameraParameter.SCENE_MODE_SUNSET;
            case CONTROL_SCENE_MODE_THEATRE:
                return CameraParameter.SCENE_MODE_THEATRE;
            case 18: //CONTROL_SCENE_MODE_HDR:
                return CameraParameter.SCENE_MODE_HDR;
            // TODO: We cannot expose FACE_PRIORITY, or HIGH_SPEED_VIDEO
        }

        Log.w(TAG, "Unable to convert from API 2 scene mode: " + sm);
        return null;
    }


    public static String whiteBalanceFromMeta(int wb) {
        switch (wb) {
            case CONTROL_AWB_MODE_AUTO:
                return CameraParameter.WHITE_BALANCE_AUTO;
            case CONTROL_AWB_MODE_CLOUDY_DAYLIGHT:
                return CameraParameter.WHITE_BALANCE_CLOUDY_DAYLIGHT;
            case CONTROL_AWB_MODE_DAYLIGHT:
                return CameraParameter.WHITE_BALANCE_DAYLIGHT;
            case CONTROL_AWB_MODE_FLUORESCENT:
                return CameraParameter.WHITE_BALANCE_FLUORESCENT;
            case CONTROL_AWB_MODE_INCANDESCENT:
                return CameraParameter.WHITE_BALANCE_INCANDESCENT;
            case CONTROL_AWB_MODE_SHADE:
                return CameraParameter.WHITE_BALANCE_SHADE;
            case CONTROL_AWB_MODE_TWILIGHT:
                return CameraParameter.WHITE_BALANCE_TWILIGHT;
            case CONTROL_AWB_MODE_WARM_FLUORESCENT:
                return CameraParameter.WHITE_BALANCE_FLUORESCENT;
        }
        Log.w(TAG, "Unable to convert from API 2 white balance: " + wb);
        return null;
    }

    public static String flashModeFromMeta(int flash) {

        switch (flash) {

            case FLASH_MODE_OFF:
                return CameraParameter.FLASH_MODE_OFF;
            case FLASH_MODE_TORCH:
                return CameraParameter.FLASH_MODE_TORCH;
            case FLASH_MODE_SINGLE:
                return CameraParameter.FLASH_MODE_ON;
        }
        return CameraParameter.FLASH_MODE_OFF;
    }
}
