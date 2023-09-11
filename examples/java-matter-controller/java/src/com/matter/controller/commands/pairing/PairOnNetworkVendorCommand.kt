/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairOnNetworkVendorCommand(controller: ChipDeviceController, credsIssue: CredentialsIssuer?) :
  PairingCommand(
    controller,
    "onnetwork-vendor",
    credsIssue,
    PairingModeType.ON_NETWORK,
    PairingNetworkType.NONE,
    DiscoveryFilterType.VENDOR_ID
  ) {
  override fun runCommand() {}
}
