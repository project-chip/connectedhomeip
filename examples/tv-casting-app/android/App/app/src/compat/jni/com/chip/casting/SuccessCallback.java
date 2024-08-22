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

/** @deprecated Use the APIs described in /examples/tv-casting-app/APIs.md instead. */
@Deprecated
public abstract class SuccessCallback<R> {
  private static final String TAG = SuccessCallback.class.getSimpleName();

  public abstract void handle(R response);

  protected final void handleInternal(R response) {
    try {
      handle(response);
    } catch (Throwable t) {
      Log.e(TAG, "SuccessCallback::Caught an unhandled Throwable from the client: " + t);
    }
  }
}
