/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.chip.casting;

import android.util.Log;

public abstract class MatterCallbackHandler {
  private static final String TAG = MatterCallbackHandler.class.getSimpleName();

  public abstract void handle(MatterError err);

  protected final void handleInternal(int errorCode, String errorMessage) {
    try {
      handle(new MatterError(errorCode, errorMessage));
    } catch (Throwable t) {
      Log.e(TAG, "MatterCallbackHandler::Caught an unhandled Throwable from the client: " + t);
    }
  }
}
