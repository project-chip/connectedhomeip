/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairAlreadyDiscoveredCommand(
  controller: ChipDeviceController,
  credsIssue: CredentialsIssuer?
) :
  PairingCommand(
    controller,
    "already-discovered",
    credsIssue,
    PairingModeType.ALREADY_DISCOVERED,
    PairingNetworkType.NONE
  ) {
  override fun runCommand() {
    currentCommissioner()
      .pairDeviceWithAddress(
        getNodeId(),
        getRemoteAddr().getHostAddress(),
        getRemotePort(),
        getDiscriminator(),
        getSetupPINCode(),
        null
      )
    currentCommissioner().setCompletionListener(this)
    waitCompleteMs(getTimeoutMillis())
  }
}
