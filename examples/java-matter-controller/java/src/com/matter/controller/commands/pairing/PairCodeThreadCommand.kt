package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairCodeThreadCommand(controller: ChipDeviceController, credsIssue: CredentialsIssuer?) :
  PairingCommand(controller, "code-thread", PairingModeType.CODE, PairingNetworkType.THREAD, credsIssue) {
  override fun runCommand() {}
}