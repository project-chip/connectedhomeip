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

public class MediaInputManagerStub implements MediaInputManager {

  private final String TAG = MediaInputManagerStub.class.getSimpleName();

  private int endpoint;
  private MediaInputInfo[] info = new MediaInputInfo[3];
  private int currentInput = 1;

  public MediaInputManagerStub(int endpoint) {
    this.endpoint = endpoint;

    info[0] = new MediaInputInfo();
    info[0].name = "HDMI 1";
    info[0].description = "High-Definition Multimedia Interface";
    info[0].index = 0;
    info[0].type = MediaInputInfo.INPUT_TYPE_HDMI;

    info[1] = new MediaInputInfo();
    info[1].name = "HDMI 2";
    info[1].description = "High-Definition Multimedia Interface";
    info[1].index = 1;
    info[1].type = MediaInputInfo.INPUT_TYPE_HDMI;

    info[2] = new MediaInputInfo();
    info[2].name = "HDMI 3";
    info[2].description = "High-Definition Multimedia Interface";
    info[2].index = 2;
    info[2].type = MediaInputInfo.INPUT_TYPE_HDMI;
  }

  @Override
  public MediaInputInfo[] getInputList() {
    return info;
  }

  @Override
  public int getCurrentInput() {
    Log.d(TAG, "getCurrentInput at " + endpoint);
    return currentInput;
  }

  @Override
  public boolean selectInput(int index) {
    Log.d(TAG, "selectInput:" + index + " at " + endpoint);
    if (index < 0 || index >= info.length) {
      return false;
    }
    currentInput = index;
    return true;
  }

  @Override
  public boolean showInputStatus() {
    Log.d(TAG, "showInputStatus at " + endpoint);
    for (MediaInputInfo mii : info) {
      Log.d(
          TAG,
          " ["
              + mii.index
              + "] type="
              + mii.type
              + " selected="
              + (currentInput == mii.index ? 1 : 0)
              + " name="
              + (mii.name == null ? "null" : mii.name)
              + " desc="
              + (mii.description == null ? "null" : mii.description));
    }
    return true;
  }

  @Override
  public boolean hideInputStatus() {
    Log.d(TAG, "hideInputStatus at " + endpoint);
    return true;
  }

  @Override
  public boolean renameInput(int index, String name) {
    Log.d(TAG, "renameInput index:" + index + " name:" + name + " at " + endpoint);
    if (index < 0 || index >= info.length) {
      return false;
    }
    info[index].name = name;
    return true;
  }
}
