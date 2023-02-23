package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairCodeCommand(controller: ChipDeviceController, credsIssue: CredentialsIssuer?) :
  PairingCommand(controller, "code", PairingModeType.CODE, PairingNetworkType.NONE, credsIssue) {
  override fun runCommand() {}
}