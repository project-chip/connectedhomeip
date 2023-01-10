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

import chip.testing.MessagingContext;

/** Utilities for unit testing {@link GetConnectedDeviceCallbackJni}. */
public final class GetConnectedDeviceCallbackForTestJni {
  private final MessagingContext messagingContext;

  static {
    System.loadLibrary("CHIPForTestController");
  }

  public GetConnectedDeviceCallbackForTestJni(MessagingContext messagingContext) {
    this.messagingContext = messagingContext;
  }

  public void onDeviceConnected(GetConnectedDeviceCallbackJni callback) {
    onDeviceConnected(callback.getCallbackHandle(), messagingContext.getMessagingContextHandle());
  }

  private native void onDeviceConnected(long callbackHandle, long messagingContextHandle);

  public void onDeviceConnectionFailure(GetConnectedDeviceCallbackJni callback, int errorCode) {
    onDeviceConnectionFailure(callback.getCallbackHandle(), errorCode);
  }

  private native void onDeviceConnectionFailure(long callbackHandle, int errorCode);
}
