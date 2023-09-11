/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.platform;

public class BleConnectCallback {
  private long implPtr;
  private long appStatePtr;

  public BleConnectCallback(long implPtr, long appStatePtr) {
    this.implPtr = implPtr;
    this.appStatePtr = appStatePtr;
  }

  public void onConnectSuccess(int connectionId) {
    onConnectSuccess(implPtr, appStatePtr, connectionId);
  }

  public void onConnectFailed() {
    onConnectFailed(implPtr, appStatePtr);
  }

  private native void onConnectSuccess(long implPtr, long appStatePtr, int connectionId);

  private native void onConnectFailed(long implPtr, long appStatePtr);
}
