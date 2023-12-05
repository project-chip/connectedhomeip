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
