/**
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
 */

package com.matter.casting.support;

import com.matter.casting.support.CommissionerDeclaration;
import com.matter.casting.support.MatterCallback;
import com.matter.casting.support.MatterError;

/**
 * @brief A container struct for User Directed Commissioning (UDC) callbacks.
 */
public class ConnectionCallbacks {

    /**
     * The callback called when the connection is established successfully.
     */
  public final MatterCallback<Void> onSuccess;

    /**
     * The callback called with MatterError when the connection is fails to establish.
     */
  public final MatterCallback<MatterError> onFailure;

    /**
     * The callback called when the Commissionee receives a CommissionerDeclaration message from the Commissioner.
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
