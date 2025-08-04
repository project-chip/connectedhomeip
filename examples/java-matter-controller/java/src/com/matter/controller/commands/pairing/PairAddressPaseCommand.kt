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
 *
 */
package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer
import java.util.logging.Level
import java.util.logging.Logger

class PairAddressPaseCommand(controller: ChipDeviceController, credsIssue: CredentialsIssuer?) :
  PairingCommand(
    controller,
    "address-paseonly",
    credsIssue,
    PairingModeType.ADDRESS_PASE_ONLY,
    PairingNetworkType.NONE
  ) {
  override fun onPairingComplete(errorCode: Long) {
    logger.log(Level.INFO, "onPairingComplete with error code: $errorCode")
    if (errorCode == 0L) {
      setSuccess()
    } else {
      setFailure("onPairingComplete failure")
    }
  }

  override fun runCommand() {
    currentCommissioner()
      .establishPaseConnection(
        getNodeId(),
        getRemoteAddr().address.hostAddress,
        getRemotePort(),
        getSetupPINCode()
      )
    currentCommissioner().setCompletionListener(this)
    waitCompleteMs(getTimeoutMillis())
  }

  companion object {
    private val logger = Logger.getLogger(PairAddressPaseCommand::class.java.name)
  }
}
