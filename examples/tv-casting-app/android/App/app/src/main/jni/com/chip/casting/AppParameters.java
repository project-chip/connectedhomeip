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

public class AppParameters {
  public static final int MIN_ROTATING_DEVICE_ID_UNIQUE_ID_LENGTH = 16;
  private static final int TEST_SETUP_PASSCODE = 20202021;
  private static final int TEST_DISCRIMINATOR = 0xF00;
  private DACProvider TEST_DAC_PROVIDER = new DACProviderStub();

  private byte[] rotatingDeviceIdUniqueId;
  private DACProvider dacProvider = TEST_DAC_PROVIDER;
  private String spake2pVerifierBase64;
  private String Spake2pSaltBase64;
  private int spake2pIterationCount;
  private int setupPasscode = TEST_SETUP_PASSCODE;
  private int discriminator = TEST_DISCRIMINATOR;

  public void setRotatingDeviceIdUniqueId(byte[] rotatingDeviceIdUniqueId) {
    this.rotatingDeviceIdUniqueId = rotatingDeviceIdUniqueId;
  }

  public byte[] getRotatingDeviceIdUniqueId() {
    return rotatingDeviceIdUniqueId;
  }

  public DACProvider getDacProvider() {
    return dacProvider;
  }

  public void setDacProvider(DACProvider dacProvider) {
    this.dacProvider = dacProvider;
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
