package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.model.NodeState
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import com.matter.controller.commands.common.CredentialsIssuer
import java.util.Collections
import java.util.logging.Level
import java.util.logging.Logger

class PairOnNetworkLongImReadCommand(
  controller: ChipDeviceController, credsIssue: CredentialsIssuer?
) : PairingCommand(
  controller,
  "onnetwork-long-im-read",
  credsIssue,
  PairingModeType.ON_NETWORK,
  PairingNetworkType.NONE,
  DiscoveryFilterType.LONG_DISCRIMINATOR
) {
  private var devicePointer: Long = 0

  private inner class InternalReportCallback : ReportCallback {
    override fun onError(attributePath: ChipAttributePath?, eventPath: ChipEventPath?, e: Exception) {
      logger.log(Level.INFO, "Read receive onError")
      setFailure("read failure")
    }

    override fun onReport(nodeState: NodeState) {
      logger.log(Level.INFO, "Read receve onReport")
      setSuccess()
    }
    
  }

  private inner class InternalGetConnectedDeviceCallback : GetConnectedDeviceCallback {
    override fun onDeviceConnected(devicePointer: Long) {
      this@PairOnNetworkLongImReadCommand.devicePointer = devicePointer
      logger.log(Level.INFO, "onDeviceConnected")
    }

    override fun onConnectionFailure(nodeId: Long, error: Exception?) {
      logger.log(Level.INFO, "onConnectionFailure")
    }
  }

  override fun runCommand() {
    val attributePathList = listOf(ChipAttributePath.newInstance(
        /* endpointId= */ 0, CLUSTER_ID_BASIC, ATTR_ID_LOCAL_CONFIG_DISABLED))

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
    currentCommissioner()
      .getConnectedDevicePointer(getNodeId(), InternalGetConnectedDeviceCallback())
    clear()
    currentCommissioner()
      .readPath(InternalReportCallback(), devicePointer, attributePathList, Collections.emptyList(), false, 0)
    waitCompleteMs(getTimeoutMillis())
  }

  companion object {
    private val logger = Logger.getLogger(
      PairOnNetworkLongImReadCommand::class.java.name
    )

    private const val MATTER_PORT = 5540
    private const val CLUSTER_ID_BASIC = 0x0028L
    private const val ATTR_ID_LOCAL_CONFIG_DISABLED = 16L
  }
}
