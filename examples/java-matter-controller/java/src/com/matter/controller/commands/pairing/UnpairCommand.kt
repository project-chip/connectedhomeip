package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class UnpairCommand(controller: ChipDeviceController, credsIssue: CredentialsIssuer?) :
  PairingCommand(controller, "unpair", PairingModeType.NONE, PairingNetworkType.NONE, credsIssue) {
  override fun runCommand() {}
}