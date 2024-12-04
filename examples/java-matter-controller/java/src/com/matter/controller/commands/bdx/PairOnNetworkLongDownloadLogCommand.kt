package com.matter.controller.commands.bdx

import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.DiagnosticLogType
import chip.devicecontroller.DownloadLogCallback
import com.matter.controller.commands.common.CredentialsIssuer
import com.matter.controller.commands.pairing.DiscoveryFilterType
import com.matter.controller.commands.pairing.PairingCommand
import com.matter.controller.commands.pairing.PairingModeType
import com.matter.controller.commands.pairing.PairingNetworkType
import java.io.File
import java.util.logging.Level
import java.util.logging.Logger

class PairOnNetworkLongDownloadLogCommand(
  controller: ChipDeviceController,
  credsIssue: CredentialsIssuer?
) :
  PairingCommand(
    controller,
    "onnetwork-long-downloadLog",
    credsIssue,
    PairingModeType.ON_NETWORK,
    PairingNetworkType.NONE,
    DiscoveryFilterType.LONG_DISCRIMINATOR
  ) {
  private val buffer: StringBuffer = StringBuffer()
  private val logType: StringBuffer = StringBuffer()
  private val fileName: StringBuffer = StringBuffer()

  init {
    addArgument("logType", logType, null, false)
    addArgument("fileName", fileName, null, false)
  }

  private inner class InternalDownloadLogCallback : DownloadLogCallback {
    override fun onError(fabricIndex: Int, nodeId: Long, errorCode: Long) {
      logger.log(
        Level.WARNING,
        "Error - FabricIndex : $fabricIndex, NodeID : $nodeId, errorCode : $errorCode"
      )
      setFailure("onError : $errorCode")
    }

    override fun onSuccess(fabricIndex: Int, nodeId: Long) {
      logger.log(Level.INFO, "FabricIndex : $fabricIndex, NodeID : $nodeId")
      logger.log(Level.INFO, "FileName : $fileName")

      val fileContent = File(fileName.toString()).readText()
      if (fileContent.compareTo(buffer.toString()) != 0) {
        setFailure("Invalid File Content")
        return
      }

      setSuccess()
    }

    override fun onTransferData(fabricIndex: Int, nodeId: Long, data: ByteArray): Boolean {
      buffer.append(String(data))
      return true
    }
  }

  override fun runCommand() {
    currentCommissioner()
      .pairDeviceWithAddress(
        getNodeId(),
        getRemoteAddr().address.hostAddress,
        MATTER_PORT,
        getDiscriminator(),
        getSetupPINCode(),
        null
      )
    currentCommissioner().setCompletionListener(this)
    waitCompleteMs(getTimeoutMillis())
    clear()
    logger.log(Level.INFO, "Type : $logType")
    logger.log(Level.INFO, "FileName : $fileName")
    currentCommissioner()
      .downloadLogFromNode(
        getNodeId(),
        DiagnosticLogType.value(logType.toString()),
        (getTimeoutMillis() / MS_TO_SEC).toLong(),
        InternalDownloadLogCallback()
      )
    logger.log(Level.INFO, "Waiting response : ${getTimeoutMillis()}")
    waitCompleteMs(getTimeoutMillis())
  }

  companion object {
    private val logger = Logger.getLogger(PairOnNetworkLongDownloadLogCommand::class.java.name)

    private const val MATTER_PORT = 5540
    private const val MS_TO_SEC = 1000
  }
}
