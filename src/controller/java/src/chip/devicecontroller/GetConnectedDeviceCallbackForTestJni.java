/*
 * SPDX-FileCopyrightText: 2020-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
