/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package com.matter.casting.support;

import android.util.Log;

public abstract class DataProvider<T> {
  private static final String TAG = DataProvider.class.getSimpleName();

  protected T _get() {
    T val = null;
    try {
      val = get();
    } catch (Throwable t) {
      Log.e(TAG, "DataProvider::Caught an unhandled Throwable from the client: " + t);
    }
    return val;
  }

  public abstract T get();
}
