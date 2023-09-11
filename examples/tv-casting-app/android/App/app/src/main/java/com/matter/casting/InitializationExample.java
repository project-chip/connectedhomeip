/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package com.matter.casting;

import android.content.Context;
import android.util.Log;
import chip.platform.ConfigurationManager;
import com.chip.casting.util.PreferencesConfigurationManager;
import com.matter.casting.core.CastingApp;
import com.matter.casting.support.AppParameters;
import com.matter.casting.support.CommissionableData;
import com.matter.casting.support.DACProvider;
import com.matter.casting.support.DataProvider;
import com.matter.casting.support.MatterError;

public class InitializationExample {
  private static final String TAG = InitializationExample.class.getSimpleName();

  /**
   * DataProvider implementation for the Unique ID that is used by the SDK to generate the Rotating
   * Device ID
   */
  private static final DataProvider<byte[]> rotatingDeviceIdUniqueIdProvider =
      new DataProvider<byte[]>() {
        private static final String ROTATING_DEVICE_ID_UNIQUE_ID =
            "EXAMPLE_ID"; // dummy value for demonstration only

        @Override
        public byte[] get() {
          return ROTATING_DEVICE_ID_UNIQUE_ID.getBytes();
        }
      };

  /**
   * DataProvider implementation for the Commissioning Data used by the SDK when the CastingApp goes
   * through commissioning
   */
  private static final DataProvider<CommissionableData> commissionableDataProvider =
      new DataProvider<CommissionableData>() {
        @Override
        public CommissionableData get() {
          // dummy values for demonstration only
          return new CommissionableData(20202021, 3874);
        }
      };

  /**
   * DACProvider implementation for the Device Attestation Credentials required at the time of
   * commissioning
   *
   * <p>Using the DACProviderStub which provides dummy values for demonstration only
   */
  private static final DACProvider dacProvider = new DACProviderStub();

  /**
   * @param applicationContext Given android.content.Context, initialize and start the CastingApp
   */
  public static MatterError initAndStart(Context applicationContext) {
    // Create an AppParameters object to pass in global casting parameters to the SDK
    final AppParameters appParameters =
        new AppParameters(
            applicationContext,
            new DataProvider<ConfigurationManager>() {
              @Override
              public ConfigurationManager get() {
                return new PreferencesConfigurationManager(
                    applicationContext, "chip.platform.ConfigurationManager");
              }
            },
            rotatingDeviceIdUniqueIdProvider,
            commissionableDataProvider,
            dacProvider);

    // Initialize the SDK using the appParameters and check if it returns successfully
    MatterError err = CastingApp.getInstance().initialize(appParameters);
    if (err.hasError()) {
      Log.e(TAG, "Failed to initialize Matter CastingApp");
      return err;
    }

    err = CastingApp.getInstance().start();
    if (err.hasError()) {
      Log.e(TAG, "Failed to start Matter CastingApp");
      return err;
    }
    return err;
  }
}
