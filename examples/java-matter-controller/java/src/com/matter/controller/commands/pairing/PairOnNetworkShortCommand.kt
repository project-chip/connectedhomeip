package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairOnNetworkShortCommand(controller: ChipDeviceController, credsIssue: CredentialsIssuer?) : PairingCommand(
  controller,
  "onnetwork-short",
  PairingModeType.ON_NETWORK,
  PairingNetworkType.NONE,
  credsIssue,
  DiscoveryFilterType.SHORT_DISCRIMINATOR
) {
  override fun runCommand() {}
}