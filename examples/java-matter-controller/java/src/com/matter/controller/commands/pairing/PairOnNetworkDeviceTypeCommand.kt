package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairOnNetworkDeviceTypeCommand(
  controller: ChipDeviceController, credsIssue: CredentialsIssuer?
) : PairingCommand(
  controller,
  "onnetwork-device-type",
  PairingModeType.ON_NETWORK,
  PairingNetworkType.NONE,
  credsIssue,
  DiscoveryFilterType.DEVICE_TYPE
) {
  override fun runCommand() {}
}