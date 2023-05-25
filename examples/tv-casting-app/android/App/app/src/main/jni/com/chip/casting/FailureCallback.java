/*
 *   Copyright (c) 2022 Project CHIP Authors
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
package com.chip.casting;

import android.util.Log;

public abstract class FailureCallback {
  private static final String TAG = FailureCallback.class.getSimpleName();

  public abstract void handle(MatterError err);

  protected final void handleInternal(int errorCode, String errorMessage) {
    try {
      handle(new MatterError(errorCode, errorMessage));
    } catch (Throwable t) {
      Log.e(TAG, "FailureCallback::Caught an unhandled Throwable from the client: " + t);
    }
  }
}
