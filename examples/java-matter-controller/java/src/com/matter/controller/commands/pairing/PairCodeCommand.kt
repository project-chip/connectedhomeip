/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairCodeCommand(val controller: ChipDeviceController, credsIssue: CredentialsIssuer?) :
  PairingCommand(controller, "code", credsIssue, PairingModeType.CODE, PairingNetworkType.NONE) {
  override fun runCommand() {
    currentCommissioner()
      .pairDeviceWithCode(
        getNodeId(),
        getOnboardingPayload(),
        getDiscoverOnce(),
        getUseOnlyOnNetworkDiscovery(),
        null,
        getWifiNetworkCredentials(),
      )
    currentCommissioner().setCompletionListener(this)
    waitCompleteMs(getTimeoutMillis())

    println("Commissioner Node ID : ${controller.getControllerNodeId()}")
  }
}
