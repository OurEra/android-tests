package com.srw.camera.test;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.support.v7.app.AppCompatActivity;

public class SimpleDrawSurfaceView extends AppCompatActivity implements SurfaceHolder.Callback {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.simple_surface_draw);

        SurfaceView simple = findViewById(R.id.simple_surface);
        simple.getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int format, int width, int height) {

        Canvas canvas = surfaceHolder.getSurface().lockCanvas(null);
        // screen default vertical
        Log.i("SimpleDraw", "width " + width + " height " + height);

        Paint paint = new Paint();

        // left top quarter
        paint.setColor(0xFF00FFFF);
        canvas.drawRect(0, 0, (int) width / 2, (int) height / 2, paint);

        // right top quarter
        paint.setColor(0xFF00FF7F);
        canvas.drawRect((int) width / 2, 0, width, (int) height / 2, paint);

        // left bottom quarter
        paint.setColor(0xFFB22222);
        canvas.drawRect(0, (int) height / 2, (int) width / 2, height, paint);

        // right bottom quarter
        paint.setColor(0xFF9400D3);
        canvas.drawRect((int) width / 2, (int) height / 2, width, height, paint);

        surfaceHolder.getSurface().unlockCanvasAndPost(canvas);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }
}
