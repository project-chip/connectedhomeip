package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairOnNetworkCommissioningModeCommand(
  controller: ChipDeviceController, credsIssue: CredentialsIssuer?
) : PairingCommand(
  controller,
  "onnetwork-commissioning-mode",
  PairingModeType.ON_NETWORK,
  PairingNetworkType.NONE,
  credsIssue,
  DiscoveryFilterType.COMMISSIONING_MODE
) {
  override fun runCommand() {}
}