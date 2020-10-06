package com.example.util;

import android.graphics.Bitmap;
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

}
