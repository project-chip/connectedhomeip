package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer

class PairOnNetworkLongCommand(controller: ChipDeviceController, credsIssue: CredentialsIssuer?) : PairingCommand(
  controller,
  "onnetwork-long",
  PairingModeType.ON_NETWORK,
  PairingNetworkType.NONE,
  credsIssue,
  DiscoveryFilterType.LONG_DISCRIMINATOR
) {
  private val MATTER_PORT = 5540

  override fun runCommand() {
    currentCommissioner()
      .pairDeviceWithAddress(
        getNodeId(),
        getRemoteAddr().getHostAddress(),
        MATTER_PORT,
        getDiscriminator(),
        getSetupPINCode(),
        null
      )
    currentCommissioner().setCompletionListener(this)
    waitCompleteMs(getTimeoutMillis())
  }
}