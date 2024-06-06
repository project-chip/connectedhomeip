/**
 * Copyright (c) 2024 Project CHIP Authors All rights reserved.
 *
 * <p>Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License at
 *
 * <p>http://www.apache.org/licenses/LICENSE-2.0
 *
 * <p>Unless required by applicable law or agreed to in writing, software distributed under the
 * License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.matter.casting.support;

/** @brief A container struct for User Directed Commissioning (UDC) callbacks. */
public class ConnectionCallbacks {

  /** (Required) The callback called when the connection is established successfully. */
  public final MatterCallback<Void> onSuccess;

  /** (Required) The callback called with MatterError when the connection is fails to establish. */
  public final MatterCallback<MatterError> onFailure;

  /**
   * (Optional) The callback called when the Client/Commissionee receives a CommissionerDeclaration
   * message from the CastingPlayer/Commissioner. This callback is needed to support UDC features
   * where a reply from the Commissioner is expected. It provides information indicating the
   * Commissioner’s pre-commissioning state.
   */
  public MatterCallback<CommissionerDeclaration> onCommissionerDeclaration;

  public ConnectionCallbacks(
      MatterCallback<Void> onSuccess,
      MatterCallback<MatterError> onFailure,
      MatterCallback<CommissionerDeclaration> onCommissionerDeclaration) {
    this.onSuccess = onSuccess;
    this.onFailure = onFailure;
    this.onCommissionerDeclaration = onCommissionerDeclaration;
  }

  public MatterCallback<Void> getOnSuccess() {
    return onSuccess;
  }

  public MatterCallback<MatterError> getOnFailure() {
    return onFailure;
  }

  public MatterCallback<CommissionerDeclaration> getOnCommissionerDeclaration() {
    return onCommissionerDeclaration;
  }
}
