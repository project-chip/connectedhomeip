package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairCodePaseCommand(controller: ChipDeviceController, credsIssue: CredentialsIssuer?) : PairingCommand(
  controller,
  "code-paseonly",
  PairingModeType.CODE_PASE_ONLY,
  PairingNetworkType.NONE,
  credsIssue
) {
  override fun runCommand() {}
}