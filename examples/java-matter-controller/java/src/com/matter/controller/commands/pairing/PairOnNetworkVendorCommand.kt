package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairOnNetworkVendorCommand(controller: ChipDeviceController, credsIssue: CredentialsIssuer?) : PairingCommand(
  controller,
  "onnetwork-vendor",
  PairingModeType.ON_NETWORK,
  PairingNetworkType.NONE,
  credsIssue,
  DiscoveryFilterType.VENDOR_ID
) {
  override fun runCommand() {}
}