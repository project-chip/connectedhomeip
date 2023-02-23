package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairOnNetworkCommissionerCommand(
  controller: ChipDeviceController, credsIssue: CredentialsIssuer?
) : PairingCommand(
  controller,
  "onnetwork-commissioner",
  PairingModeType.ON_NETWORK,
  PairingNetworkType.NONE,
  credsIssue,
  DiscoveryFilterType.COMMISSIONER
) {
  override fun runCommand() {}
}