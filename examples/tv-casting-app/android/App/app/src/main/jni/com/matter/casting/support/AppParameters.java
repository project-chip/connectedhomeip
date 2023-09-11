/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package com.matter.casting.support;

import android.content.Context;
import androidx.annotation.NonNull;
import chip.platform.ConfigurationManager;

public class AppParameters {
  @NonNull private final Context applicationContext;

  @NonNull private final DataProvider<ConfigurationManager> configurationManagerProvider;

  @NonNull private final DataProvider<byte[]> rotatingDeviceIdUniqueIdProvider;

  @NonNull private final DataProvider<CommissionableData> commissionableDataProvider;

  @NonNull private final DACProvider dacProvider;

  /**
   * @param applicationContext the Android app's android.content.Context
   * @param configurationManagerProvider Implementation of chip.platform.ConfigurationManager
   * @param rotatingDeviceIdUniqueIdProvider Provides values of the uniqueID used to generate the
   *     RotatingDeviceId of the CastingApp
   * @param commissionableDataProvider Provides CommissionableData (such as setupPasscode,
   *     discriminator, etc) used to get the CastingApp commissioned
   * @param dacProvider Provides DeviceAttestationCredentials of the CastingApp during commissioning
   */
  public AppParameters(
      @NonNull Context applicationContext,
      @NonNull DataProvider<ConfigurationManager> configurationManagerProvider,
      @NonNull DataProvider<byte[]> rotatingDeviceIdUniqueIdProvider,
      @NonNull DataProvider<CommissionableData> commissionableDataProvider,
      @NonNull DACProvider dacProvider) {
    this.applicationContext = applicationContext;
    this.configurationManagerProvider = configurationManagerProvider;
    this.rotatingDeviceIdUniqueIdProvider = rotatingDeviceIdUniqueIdProvider;
    this.commissionableDataProvider = commissionableDataProvider;
    this.dacProvider = dacProvider;
  }

  @NonNull
  public Context getApplicationContext() {
    return applicationContext;
  }

  @NonNull
  public DataProvider<ConfigurationManager> getConfigurationManagerProvider() {
    return configurationManagerProvider;
  }

  @NonNull
  public DataProvider<byte[]> getRotatingDeviceIdUniqueIdProvider() {
    return rotatingDeviceIdUniqueIdProvider;
  }

  @NonNull
  public DataProvider<CommissionableData> getCommissionableDataProvider() {
    return commissionableDataProvider;
  }

  @NonNull
  public DACProvider getDacProvider() {
    return dacProvider;
  }
}
