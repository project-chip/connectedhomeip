package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairCodeWifiCommand(controller: ChipDeviceController, credsIssue: CredentialsIssuer?) :
  PairingCommand(controller, "code-wifi", PairingModeType.CODE, PairingNetworkType.WIFI, credsIssue) {
  override fun runCommand() {}
}