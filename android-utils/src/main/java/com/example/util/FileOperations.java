package com.example.util;

import android.graphics.Bitmap;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class FileOperations {

    private static final String TAG = "UTILS-" + FileOperations.class.getSimpleName();

    public static void saveBitmapToFile(Bitmap bitmap, String path) {

        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        bitmap.compress(Bitmap.CompressFormat.JPEG, 100, baos);
        File file = new File(path);
        try {
            FileOutputStream fos = new FileOutputStream(file);
            try {
                fos.write(baos.toByteArray());
                fos.flush();
                fos.close();
            } catch (IOException e) {
                Log.e(TAG, "err: " + e.getMessage());
            }
        } catch (FileNotFoundException e) {
            Log.e(TAG, "err: " + e.getMessage());
        }
    }
}
