/*
 *   Copyright (c) 2023 Project CHIP Authors
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
 */

package com.matter.casting.support;

import android.util.Log;

public interface DataProvider<T> {
  public static final String TAG = DataProvider.class.getSimpleName();

  default T _get() {
    T val = null;
    try {
      val = get();
    } catch (Throwable t) {
      Log.e(TAG, "DataProvider::Caught an unhandled Throwable from the client: " + t);
    }
    return val;
  }

  public abstract T get();

  /**
   * Must be implemented in the CommissionableData DataProvider if the Commissioner-Generated
   * passcode commissioning flow is going to be used. In this flow, the setup passcode is generated
   * by the CastingPlayer and entered by the user in the tv-casting-app CX. Once it is obtained,
   * this function should be called with the Commissioner-Generated passcode to update the
   * CommissionableData DataProvider in AppParameters. The client is also responsible for calling
   * CastingApp::updateAndroidChipPlatformWithCommissionableData() to update the data provider set
   * which was previously set in the AppParameters and AndroidChipPlatform at initialization time.
   */
  default void updateCommissionableDataSetupPasscode(long setupPasscode, int discriminator) {}
}
