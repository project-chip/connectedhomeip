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

package com.matter.casting.core;

import android.content.Context;
import android.util.Log;
import chip.appserver.ChipAppServer;
import chip.platform.AndroidBleManager;
import chip.platform.AndroidChipPlatform;
import chip.platform.ChipMdnsCallbackImpl;
import chip.platform.DiagnosticDataProviderImpl;
import chip.platform.NsdManagerServiceBrowser;
import chip.platform.NsdManagerServiceResolver;
import chip.platform.PreferencesKeyValueStoreManager;
import com.matter.casting.support.AppParameters;
import com.matter.casting.support.CommissionableData;
import com.matter.casting.support.MatterError;

/**
 * CastingApp represents an app that can cast content to a Casting Player. This class is a
 * singleton.
 */
public final class CastingApp {
  private static final String TAG = CastingApp.class.getSimpleName();
  private static final long BROWSE_SERVICE_TIMEOUT = 2500;
  private static final long RESOLVE_SERVICE_TIMEOUT = 3000;

  private static CastingApp sInstance;

  private CastingAppState mState = CastingAppState.UNINITIALIZED;
  private AppParameters appParameters;
  private NsdManagerServiceResolver.NsdManagerResolverAvailState nsdManagerResolverAvailState;
  private ChipAppServer chipAppServer;
  private AndroidChipPlatform chipPlatform;

  private CastingApp() {}

  public static CastingApp getInstance() {
    if (sInstance == null) {
      sInstance = new CastingApp();
    }
    return sInstance;
  }

  /**
   * Initializes the CastingApp with appParameters
   *
   * @param appParameters
   */
  public MatterError initialize(AppParameters appParameters) {
    Log.i(TAG, "CastingApp.initialize() called");
    if (mState != CastingAppState.UNINITIALIZED) {
      return MatterError.CHIP_ERROR_INCORRECT_STATE;
    }

    this.appParameters = appParameters;
    this.nsdManagerResolverAvailState =
        new NsdManagerServiceResolver.NsdManagerResolverAvailState();

    Context applicationContext = appParameters.getApplicationContext();
    chipPlatform =
        new AndroidChipPlatform(
            new AndroidBleManager(),
            new PreferencesKeyValueStoreManager(appParameters.getApplicationContext()),
            appParameters.getConfigurationManagerProvider().get(),
            new NsdManagerServiceResolver(
                applicationContext, nsdManagerResolverAvailState, RESOLVE_SERVICE_TIMEOUT),
            new NsdManagerServiceBrowser(applicationContext, BROWSE_SERVICE_TIMEOUT),
            new ChipMdnsCallbackImpl(),
            new DiagnosticDataProviderImpl(applicationContext));

    MatterError err = updateAndroidChipPlatformWithCommissionableData();
    if (err.hasError()) {
      Log.e(
          TAG,
          "CastingApp.initialize() failed to updateCommissionableDataProviderData() on AndroidChipPlatform");
      return err;
    }

    err = finishInitialization(appParameters);

    if (err.hasNoError()) {
      chipAppServer = new ChipAppServer(); // get a reference to the Matter server now
      mState = CastingAppState.NOT_RUNNING; // initialization done, set state to NOT_RUNNING
    }
    return err;
  }

  /**
   * Updates the Android CHIP platform with the CommissionableData. This function retrieves
   * commissionable data from the AppParameters and updates the Android CHIP platform using this
   * data. The commissionable data includes information such as the SPAKE2+ verifier, salt,
   * iteration count, setup passcode, and discriminator.
   *
   * @return MatterError.NO_ERROR if the update was successful,
   *     MatterError.CHIP_ERROR_INVALID_ARGUMENT otherwise.
   */
  MatterError updateAndroidChipPlatformWithCommissionableData() {
    Log.i(TAG, "CastingApp.updateAndroidChipPlatformWithCommissionableData()");
    CommissionableData commissionableData = appParameters.getCommissionableDataProvider().get();
    boolean updated =
        chipPlatform.updateCommissionableDataProviderData(
            commissionableData.getSpake2pVerifierBase64(),
            commissionableData.getSpake2pSaltBase64(),
            commissionableData.getSpake2pIterationCount(),
            commissionableData.getSetupPasscode(),
            commissionableData.getDiscriminator());
    if (!updated) {
      Log.e(
          TAG,
          "CastingApp.updateAndroidChipPlatformWithCommissionableData() failed to updateCommissionableDataProviderData() on AndroidChipPlatform");
      return MatterError.CHIP_ERROR_INVALID_ARGUMENT;
    }
    return MatterError.NO_ERROR;
  }

  /**
   * Starts the Matter server that the CastingApp runs on and registers all the necessary delegates
   */
  public MatterError start() {
    Log.i(TAG, "CastingApp.start called");
    if (mState != CastingAppState.NOT_RUNNING) {
      return MatterError.CHIP_ERROR_INCORRECT_STATE;
    }

    boolean serverStarted = chipAppServer.startApp();
    if (!serverStarted) {
      Log.e(TAG, "CastingApp.start failed to start Matter server");
      return MatterError.CHIP_ERROR_INCORRECT_STATE;
    }

    MatterError err = finishStartup();
    if (err.hasNoError()) {
      mState = CastingAppState.RUNNING; // CastingApp started successfully, set state to RUNNING
    }
    return err;
  }

  /**
   * Stops the Matter server that the CastingApp runs on
   *
   * @return
   */
  public MatterError stop() {
    Log.i(TAG, "CastingApp.stop called");
    if (mState != CastingAppState.RUNNING) {
      return MatterError.CHIP_ERROR_INCORRECT_STATE;
    }

    boolean serverStopped = chipAppServer.stopApp();
    if (!serverStopped) {
      Log.e(TAG, "CastingApp.stop failed to stop Matter server");
      return MatterError.CHIP_ERROR_INCORRECT_STATE;
    }
    mState =
        CastingAppState.NOT_RUNNING; // CastingApp stopped successfully, set state to NOT_RUNNING

    return MatterError.NO_ERROR;
  }

  /** @brief Tears down all active subscriptions. */
  public native MatterError shutdownAllSubscriptions();

  /**
   * Clears app cache that contains the information about CastingPlayers previously connected to
   *
   * @return
   */
  public native MatterError clearCache();

  /**
   * Sets DeviceAttestationCrdentials provider and RotatingDeviceIdUniqueId
   *
   * @param appParameters
   */
  private native MatterError finishInitialization(AppParameters appParameters);

  /** Performs post Matter server startup registrations */
  private native MatterError finishStartup();

  static {
    System.loadLibrary("TvCastingApp");
  }
}
