/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package com.matter.casting.support;

import androidx.annotation.Nullable;

public class CommissionableData {
  private long setupPasscode;

  private int discriminator;

  @Nullable private String spake2pVerifierBase64;

  @Nullable private String spake2pSaltBase64;

  private int spake2pIterationCount;

  public CommissionableData(long setupPasscode, int discriminator) {
    this.setupPasscode = setupPasscode;
    this.discriminator = discriminator;
  }

  public long getSetupPasscode() {
    return setupPasscode;
  }

  public int getDiscriminator() {
    return discriminator;
  }

  @Nullable
  public String getSpake2pVerifierBase64() {
    return spake2pVerifierBase64;
  }

  public void setSpake2pVerifierBase64(@Nullable String spake2pVerifierBase64) {
    this.spake2pVerifierBase64 = spake2pVerifierBase64;
  }

  @Nullable
  public String getSpake2pSaltBase64() {
    return spake2pSaltBase64;
  }

  public void setSpake2pSaltBase64(@Nullable String spake2pSaltBase64) {
    this.spake2pSaltBase64 = spake2pSaltBase64;
  }

  public int getSpake2pIterationCount() {
    return spake2pIterationCount;
  }

  public void setSpake2pIterationCount(int spake2pIterationCount) {
    this.spake2pIterationCount = spake2pIterationCount;
  }
}
