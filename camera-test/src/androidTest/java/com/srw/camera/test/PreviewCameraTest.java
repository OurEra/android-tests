package com.srw.camera.test;

import android.os.Build;
import static android.support.test.InstrumentationRegistry.getInstrumentation;
import static android.support.test.InstrumentationRegistry.getTargetContext;
import static android.support.test.espresso.Espresso.onView;
import static android.support.test.espresso.action.ViewActions.closeSoftKeyboard;
import static android.support.test.espresso.action.ViewActions.typeText;
import static android.support.test.espresso.matcher.ViewMatchers.withId;

import android.support.test.rule.ActivityTestRule;
import android.support.test.runner.AndroidJUnit4;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
public class PreviewCameraTest {

    @Rule
    public ActivityTestRule<CameraPreview> mActivityRule = new ActivityTestRule<>(
            CameraPreview.class);

    @Test
    public void changeText() {
        HoldPage();
        onView(withId(R.id.preview_tx))
                .perform(typeText("2134"), closeSoftKeyboard());
        HoldPage();
    }

    @Before
    public void grantPhonePermission() {
        // In M+, trying to call a number will trigger a runtime dialog. Make sure
        // the permission is granted before running this test.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            getInstrumentation().getUiAutomation().executeShellCommand(
                    "pm grant " + getTargetContext().getPackageName()
                            + " android.permission.CAMERA");
        }
    }

    private void HoldPage() {
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
