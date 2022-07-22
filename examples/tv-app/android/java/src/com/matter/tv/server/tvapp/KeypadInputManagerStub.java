/*
 *   Copyright (c) 2021 Project CHIP Authors
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
