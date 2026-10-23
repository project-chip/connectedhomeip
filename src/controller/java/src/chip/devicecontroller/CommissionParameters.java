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

  /**
   * The information for ICD registration. For detailed information see {@link ICDRegistrationInfo}.
   *
   * <p>Behavior depends on the commissioning mode and how this value is set:
   *
   * <ul>
   *   <li><b>LIT mode, immediate configuration</b>: If LIT mode for commissioning is used and
   *       {@link CompletionListener.onICDRegistrationInfoRequired} is not needed, all required
   *       values in {@link ICDRegistrationInfo} must be provided directly.
   *   <li><b>LIT mode, deferred configuration</b>: If LIT mode for commissioning is used and {@link
   *       CompletionListener.onICDRegistrationInfoRequired} should be invoked to obtain
   *       registration information later, set this value to an instance created with {@link
   *       ICDRegistrationInfo#createForDeferredConfiguration()}.
   *   <li><b>No ICD registration (e.g., SIT mode)</b>: If ICD registration is not used, set this
   *       value to {@code null}. A {@code null} value results in {@code
   *       ICDRegistrationStrategy::kIgnore} and will <b>not</b> cause {@link
   *       CompletionListener.onICDRegistrationInfoRequired} to be called.
   * </ul>
   */
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
