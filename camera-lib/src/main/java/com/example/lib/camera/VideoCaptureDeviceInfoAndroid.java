package com.example.lib.camera;

import java.util.List;

import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.Size;
import android.hardware.Camera;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class VideoCaptureDeviceInfoAndroid{
  private final static String TAG = VideoCaptureDeviceInfoAndroid.class.getSimpleName();

  private static boolean isFrontFacing(CameraInfo info) {
    return info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT;
  }

  private static String deviceUniqueName(int index, CameraInfo info) {
    return "Camera " + index +", Facing " +
        (isFrontFacing(info) ? "front" : "back") +
        ", Orientation "+ info.orientation;
  }

  // Returns information about all cameras on the device as a serialized JSON
  // array of dictionaries encoding information about a single device.  Since
  // this reflects static information about the hardware present, there is no
  // need to call this function more than once in a single process.  It is
  // marked "private" as it is only called by native code.
  public static String getDeviceInfo() {
    try {
      JSONArray devices = new JSONArray();
      for (int i = 0; i < Camera.getNumberOfCameras(); ++i) {
        CameraInfo info = new CameraInfo();
        Camera.getCameraInfo(i, info);
        String uniqueName = deviceUniqueName(i, info);
        JSONObject cameraDict = new JSONObject();
        devices.put(cameraDict);
        List<Size> supportedSizes;
        List<int[]> supportedFpsRanges;
        Camera camera = null;
        try {
          camera = Camera.open(i);
          Parameters parameters = camera.getParameters();
          supportedSizes = parameters.getSupportedPreviewSizes();
          supportedFpsRanges = parameters.getSupportedPreviewFpsRange();
          Log.d(TAG,  "getDeviceInfo " + uniqueName);
        } catch (RuntimeException e) {
          Log.e(TAG, "Failed to open " + uniqueName + ", skipping", e);
          continue;
        } finally {
          if (camera != null) {
            camera.release();
          }
        }

        JSONArray sizes = new JSONArray();
        for (Size supportedSize : supportedSizes) {
          JSONObject size = new JSONObject();
          size.put("width", supportedSize.width);
          size.put("height", supportedSize.height);
          sizes.put(size);
        }

        JSONArray mfpsRanges = new JSONArray();
        for (int[] range : supportedFpsRanges) {
          JSONObject mfpsRange = new JSONObject();
          // Android SDK deals in integral "milliframes per second"
          // (i.e. fps*1000, instead of floating-point frames-per-second) so we
          // preserve that through the Java->C++->Java round-trip.
          mfpsRange.put("min_mfps", range[Parameters.PREVIEW_FPS_MIN_INDEX]);
          mfpsRange.put("max_mfps", range[Parameters.PREVIEW_FPS_MAX_INDEX]);
          mfpsRanges.put(mfpsRange);
        }

        cameraDict.put("name", uniqueName);
        cameraDict.put("front_facing", isFrontFacing(info))
            .put("orientation", info.orientation)
            .put("sizes", sizes)
            .put("mfpsRanges", mfpsRanges);
      }
      String ret = devices.toString(2);
      Log.d(TAG, ret);
      return ret;
    } catch (JSONException e) {
      throw new RuntimeException(e);
    }
  }
}
