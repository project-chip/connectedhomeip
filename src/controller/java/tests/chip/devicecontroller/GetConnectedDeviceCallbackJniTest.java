/*
 *   Copyright (c) 2020-2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package chip.devicecontroller;

import static com.google.common.truth.Truth.assertThat;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback;
import chip.testing.MessagingContext;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
public final class GetConnectedDeviceCallbackJniTest {
  private GetConnectedDeviceCallbackForTestJni callbackTestUtil;

  @Before
  public void setUp() {
    callbackTestUtil = new GetConnectedDeviceCallbackForTestJni(new MessagingContext());
  }

  @Test
  public void deviceConnected() {
    var callback = new FakeGetConnectedDeviceCallback();
    var jniCallback = new GetConnectedDeviceCallbackJni(callback);
    callbackTestUtil.onDeviceConnected(jniCallback);

    assertThat(callback.devicePointer).isNotEqualTo(0L);
  }

  @Test
  public void connectionFailure() {
    var callback = new FakeGetConnectedDeviceCallback();
    var jniCallback = new GetConnectedDeviceCallbackJni(callback);
    callbackTestUtil.onDeviceConnectionFailure(jniCallback, 100L);

    assertThat(callback.error).isInstanceOf(ChipDeviceControllerException.class);
    assertThat(((ChipDeviceControllerException) callback.error).errorCode).isEqualTo(100L);
  }

  class FakeGetConnectedDeviceCallback implements GetConnectedDeviceCallback {
    long devicePointer = 0;
    long nodeId = 0;
    Exception error = null;

    @Override
    public void onDeviceConnected(long devicePointer) {
      this.devicePointer = devicePointer;
    }

    @Override
    public void onConnectionFailure(long nodeId, Exception error) {
      this.nodeId = nodeId;
      this.error = error;
    }
  }
}
