package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairOnNetworkInstanceNameCommand(
  controller: ChipDeviceController, credsIssue: CredentialsIssuer?
) : PairingCommand(
  controller,
  "onnetwork-instance-name",
  PairingModeType.ON_NETWORK,
  PairingNetworkType.NONE,
  credsIssue,
  DiscoveryFilterType.INSTANCE_NAME
) {
  override fun runCommand() {}
}