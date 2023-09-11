/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.chip.casting;

import android.util.Log;
import chip.platform.ConfigurationManager;
import java.math.BigInteger;
import java.util.Arrays;

public class AppParameters {
  private static final String TAG = AppParameters.class.getSimpleName();
  public static final int MIN_ROTATING_DEVICE_ID_UNIQUE_ID_LENGTH = 16;
  private static final int TEST_SETUP_PASSCODE = 20202021;
  private static final int TEST_DISCRIMINATOR = 0xF00;

  private ConfigurationManager configurationManager;
  private byte[] rotatingDeviceIdUniqueId;
  private String spake2pVerifierBase64;
  private String Spake2pSaltBase64;
  private int spake2pIterationCount;
  private int setupPasscode = TEST_SETUP_PASSCODE;
  private int discriminator = TEST_DISCRIMINATOR;

  public ConfigurationManager getConfigurationManager() {
    return configurationManager;
  }

  public void setConfigurationManager(ConfigurationManager configurationManager) {
    this.configurationManager = configurationManager;
  }

  public void setRotatingDeviceIdUniqueId(byte[] rotatingDeviceIdUniqueId) {
    Log.d(
        TAG,
        "AppParameters.setRotatingDeviceIdUniqueId called with "
            + new BigInteger(1, rotatingDeviceIdUniqueId).toString(16));
    this.rotatingDeviceIdUniqueId =
        Arrays.copyOf(rotatingDeviceIdUniqueId, rotatingDeviceIdUniqueId.length);
  }

  public byte[] getRotatingDeviceIdUniqueId() {
    Log.d(
        TAG,
        "AppParameters.getRotatingDeviceIdUniqueId returning copyOf "
            + new BigInteger(1, rotatingDeviceIdUniqueId).toString(16));
    return Arrays.copyOf(rotatingDeviceIdUniqueId, rotatingDeviceIdUniqueId.length);
  }

  public String getSpake2pVerifierBase64() {
    return spake2pVerifierBase64;
  }

  public void setSpake2pVerifierBase64(String spake2pVerifierBase64) {
    this.spake2pVerifierBase64 = spake2pVerifierBase64;
  }

  public String getSpake2pSaltBase64() {
    return Spake2pSaltBase64;
  }

  public void setSpake2pSaltBase64(String spake2pSaltBase64) {
    Spake2pSaltBase64 = spake2pSaltBase64;
  }

  public int getSpake2pIterationCount() {
    return spake2pIterationCount;
  }

  public void setSpake2pIterationCount(int spake2pIterationCount) {
    this.spake2pIterationCount = spake2pIterationCount;
  }

  public int getSetupPasscode() {
    return setupPasscode;
  }

  public void setSetupPasscode(int setupPasscode) {
    this.setupPasscode = setupPasscode;
  }

  public int getDiscriminator() {
    return discriminator;
  }

  public void setDiscriminator(int discriminator) {
    this.discriminator = discriminator;
  }
}
