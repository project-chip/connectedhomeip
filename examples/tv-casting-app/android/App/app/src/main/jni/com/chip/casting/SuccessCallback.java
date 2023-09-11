/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.chip.casting;

import android.util.Log;

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
