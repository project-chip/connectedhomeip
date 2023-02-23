package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairAlreadyDiscoveredCommand(
  controller: ChipDeviceController, credsIssue: CredentialsIssuer?
) : PairingCommand(
  controller,
  "already-discovered",
  PairingModeType.ALREADY_DISCOVERED,
  PairingNetworkType.NONE,
  credsIssue
) {
  override fun runCommand() {
    currentCommissioner()
      .pairDeviceWithAddress(
        getNodeId(),
        getRemoteAddr().getHostAddress(),
        getRemotePort(),
        getDiscriminator(),
        getSetupPINCode(),
        null
      )
    currentCommissioner().setCompletionListener(this)
    waitCompleteMs(getTimeoutMillis())
  }
}