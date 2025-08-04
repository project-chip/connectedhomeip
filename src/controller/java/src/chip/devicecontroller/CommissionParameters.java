/*
 *   Copyright (c) 2024 Project CHIP Authors
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
package chip.devicecontroller;

import javax.annotation.Nullable;

/** Parameters representing commissioning arguments for {@link ChipDeviceController}. */
public final class CommissionParameters {
  @Nullable private final byte[] csrNonce;
  @Nullable private final NetworkCredentials networkCredentials;
  @Nullable private final ICDRegistrationInfo icdRegistrationInfo;

  private CommissionParameters(Builder builder) {
    csrNonce = builder.csrNonce;
    networkCredentials = builder.networkCredentials;
    icdRegistrationInfo = builder.icdRegistrationInfo;
  }
  /* a nonce to be used for the CSR request */
  public byte[] getCsrNonce() {
    return csrNonce;
  }

  /* the credentials (Wi-Fi or Thread) to be provisioned */
  public NetworkCredentials getNetworkCredentials() {
    return networkCredentials;
  }
  /* the informations for ICD registration. For detailed information {@link ICDRegistrationInfo}. If this value is null when commissioning an ICD device, {@link CompletionListener.onICDRegistrationInfoRequired} is called to request the ICDRegistrationInfo value. */
  public ICDRegistrationInfo getICDRegistrationInfo() {
    return icdRegistrationInfo;
  }

  public static class Builder {
    @Nullable private byte[] csrNonce = null;
    @Nullable private NetworkCredentials networkCredentials = null;
    @Nullable private ICDRegistrationInfo icdRegistrationInfo = null;

    public Builder setCsrNonce(byte[] csrNonce) {
      this.csrNonce = csrNonce;
      return this;
    }

    public Builder setNetworkCredentials(NetworkCredentials networkCredentials) {
      this.networkCredentials = networkCredentials;
      return this;
    }

    public Builder setICDRegistrationInfo(ICDRegistrationInfo icdRegistrationInfo) {
      this.icdRegistrationInfo = icdRegistrationInfo;
      return this;
    }

    public CommissionParameters build() {
      return new CommissionParameters(this);
    }
  }
}
