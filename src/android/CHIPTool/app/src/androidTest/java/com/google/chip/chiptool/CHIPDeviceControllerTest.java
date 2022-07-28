package com.google.chip.chiptool;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import android.content.Context;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;
import chip.devicecontroller.ChipDeviceController;
import chip.devicecontroller.PaseVerifierParams;
import org.junit.Test;
import org.junit.runner.RunWith;

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
@RunWith(AndroidJUnit4.class)
public class CHIPDeviceControllerTest {
  @Test
  public void PaseVerifierTest() {
    long deviceId = 123L;
    long setupPincode = 808080L;
    long iterations = 1000L;
    byte[] randomSalt = "hEvzbU:%h)?aB,h7+9fn[Lf[BhYB!=TA".getBytes();

    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    ChipDeviceController chipDeviceController = ChipClient.INSTANCE.getDeviceController(appContext);

    PaseVerifierParams params =
        chipDeviceController.computePaseVerifier(deviceId, setupPincode, iterations, randomSalt);

    assertNotNull(params);
    assertEquals(params.getSetupPincode(), setupPincode);
    assertNotNull(params.getPakeVerifier());
  }
}
