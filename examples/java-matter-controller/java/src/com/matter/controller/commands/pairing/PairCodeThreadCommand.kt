/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairCodeThreadCommand(controller: ChipDeviceController, credsIssue: CredentialsIssuer?) :
  PairingCommand(
    controller,
    "code-thread",
    credsIssue,
    PairingModeType.CODE,
    PairingNetworkType.THREAD
  ) {
  override fun runCommand() {
    currentCommissioner()
      .pairDeviceWithCode(
        getNodeId(),
        getOnboardingPayload(),
        getDiscoverOnce(),
        getUseOnlyOnNetworkDiscovery(),
        null,
        getThreadNetworkCredentials(),
      )
    currentCommissioner().setCompletionListener(this)
    waitCompleteMs(getTimeoutMillis())
  }
}
