package com.example.util;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;

public class BitmapOperations {

  private static final String TAG = "UTILS-" + BitmapOperations.class.getSimpleName();

  public static Bitmap createBitmapWithString(int width, int height, String content) {
    Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
    Paint paint = new Paint();
    Canvas canvas = new Canvas(bitmap);
    canvas.drawRect(0, 0, width, height, paint);
    canvas.drawColor(Color.WHITE);

    paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC));
    paint.setTextSize(40);
    paint.setTextScaleX(1.f);
    paint.setAlpha(0);
    paint.setAntiAlias(true);
    paint.setColor(0xFF008080);
    canvas.drawText(content, 200, 200, paint);
    return bitmap;
  }

    public static int calculateSampleSize(int srcWidth, int srcHeight, int dstWidth, int dstHeight) {
        final float srcAspect = (float) srcWidth / (float) srcHeight;
        final float dstAspect = (float) dstWidth / (float) dstHeight;

        if (srcAspect > dstAspect) {
            return srcWidth / dstWidth;
        } else {
            return srcHeight / dstHeight;
        }
    }

    public static Bitmap decodeResource(Resources res, int resId, int dstWidth, int dstHeight) {
        BitmapFactory.Options options = null;
        if (dstWidth > 0 && dstHeight > 0) {
            options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeResource(res, resId, options);
            options.inScaled = true;
            options.inJustDecodeBounds = false;
            options.inSampleSize = calculateSampleSize(options.outWidth, options.outHeight, dstWidth,
                    dstHeight);
        }

        return BitmapFactory.decodeResource(res, resId, options);
    }

    public static Bitmap decodeResource(String resPath, int dstWidth, int dstHeight) {
        BitmapFactory.Options options = null;
        if (dstWidth > 0 && dstHeight > 0) {
            options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeFile(resPath, options);
            options.inJustDecodeBounds = false;
            options.inScaled = true;
            options.inSampleSize = calculateSampleSize(options.outWidth, options.outHeight, dstWidth,
                    dstHeight);
        }
        return BitmapFactory.decodeFile(resPath, options);
    }

    public static Bitmap decodeResource(Bitmap resBitmap, int dstWidth, int dstHeight) {
        if (dstWidth > 0 && dstHeight > 0) {
            return Bitmap.createScaledBitmap(resBitmap, dstWidth, dstHeight, true);
        } else {
            return resBitmap;
        }
    }

}
