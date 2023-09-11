/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairOnNetworkShortCommand(controller: ChipDeviceController, credsIssue: CredentialsIssuer?) :
  PairingCommand(
    controller,
    "onnetwork-short",
    credsIssue,
    PairingModeType.ON_NETWORK,
    PairingNetworkType.NONE,
    DiscoveryFilterType.SHORT_DISCRIMINATOR
  ) {
  override fun runCommand() {}
}
