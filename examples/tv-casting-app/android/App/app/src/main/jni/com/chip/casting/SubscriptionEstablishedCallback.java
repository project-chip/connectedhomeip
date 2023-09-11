/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.chip.casting;

import android.util.Log;

public abstract class SubscriptionEstablishedCallback {
  private static final String TAG = SubscriptionEstablishedCallback.class.getSimpleName();

  public abstract void handle();

  protected void handleInternal() {
    try {
      handle();
    } catch (Throwable t) {
      Log.e(
          TAG,
          "SubscriptionEstablishedCallback::Caught an unhandled Throwable from the client: " + t);
    }
  }
}
