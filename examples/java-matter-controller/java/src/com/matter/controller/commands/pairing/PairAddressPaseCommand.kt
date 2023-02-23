package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer
import java.util.logging.Level
import java.util.logging.Logger

class PairAddressPaseCommand(controller: ChipDeviceController, credsIssue: CredentialsIssuer?) : PairingCommand(
  controller,
  "address-paseonly",
  PairingModeType.ADDRESS_PASE_ONLY,
  PairingNetworkType.NONE,
  credsIssue
) {
  private val logger = Logger.getLogger(PairAddressPaseCommand::class.java.name)

  override fun onPairingComplete(errorCode: Int) {
    logger.log(Level.INFO, "onPairingComplete with error code: $errorCode")
    if (errorCode == 0) {
      setSuccess()
    } else {
      setFailure("onPairingComplete failure")
    }
  }

  override fun runCommand() {
    currentCommissioner()
      .establishPaseConnection(
        getNodeId(),
        getRemoteAddr().getHostAddress(),
        getRemotePort(),
        getSetupPINCode()
      )
    currentCommissioner().setCompletionListener(this)
    waitCompleteMs(getTimeoutMillis())
  }
}