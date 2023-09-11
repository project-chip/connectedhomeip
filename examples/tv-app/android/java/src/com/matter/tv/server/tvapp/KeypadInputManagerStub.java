/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.tv.server.tvapp;

import android.util.Log;

/** Stub implement of KeypadInputManager, TV manufacture should have there own implements */
public class KeypadInputManagerStub implements KeypadInputManager {

  private final String TAG = KeypadInputManagerStub.class.getSimpleName();
  private int endpoint;

  public KeypadInputManagerStub(int endpoint) {
    this.endpoint = endpoint;
  }

  @Override
  public int sendKey(int keyCode) {
    Log.d(TAG, "sendKey:" + keyCode + " at " + endpoint);

    if (keyCode == KeypadInputManager.KEY_CODE_F4_YELLOW) {
      return KEY_STATUS_UNSUPPORTED_KEY;
    }
    return KEY_STATUS_SUCCESS;
  }
}
