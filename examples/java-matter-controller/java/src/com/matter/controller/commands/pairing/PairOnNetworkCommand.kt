package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairOnNetworkCommand(controller: ChipDeviceController, credsIssue: CredentialsIssuer?) :
  PairingCommand(controller, "onnetwork", PairingModeType.ON_NETWORK, PairingNetworkType.NONE, credsIssue) {
  override fun runCommand() {}
}