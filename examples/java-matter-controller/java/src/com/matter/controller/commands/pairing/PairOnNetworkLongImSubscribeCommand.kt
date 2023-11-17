package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.ResubscriptionAttemptCallback
import chip.devicecontroller.SubscriptionEstablishedCallback
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.ChipPathId
import chip.devicecontroller.model.DataVersionFilter
import chip.devicecontroller.model.NodeState
import com.matter.controller.commands.common.CredentialsIssuer
import java.util.Collections
import java.util.logging.Level
import java.util.logging.Logger

class PairOnNetworkLongImSubscribeCommand(
  controller: ChipDeviceController,
  credsIssue: CredentialsIssuer?
) :
  PairingCommand(
    controller,
    "onnetwork-long-im-subscribe",
    credsIssue,
    PairingModeType.ON_NETWORK,
    PairingNetworkType.NONE,
    DiscoveryFilterType.LONG_DISCRIMINATOR
  ) {
  private var devicePointer: Long = 0
  private var subscriptionId: Long = 0

  private inner class InternalReportCallback : ReportCallback {
    override fun onError(
      attributePath: ChipAttributePath?,
      eventPath: ChipEventPath?,
      e: Exception
    ) {
      logger.log(Level.INFO, "Subscribe receive onError")
      setFailure("write failure")
    }

    override fun onReport(nodeState: NodeState) {
      logger.log(Level.INFO, "Subscribe receve onReport")
    }
  }

  private inner class InternalGetConnectedDeviceCallback : GetConnectedDeviceCallback {
    override fun onDeviceConnected(devicePointer: Long) {
      this@PairOnNetworkLongImSubscribeCommand.devicePointer = devicePointer
      logger.log(Level.INFO, "onDeviceConnected")
    }

    override fun onConnectionFailure(nodeId: Long, error: Exception?) {
      logger.log(Level.INFO, "onConnectionFailure")
    }
  }

  private inner class InternalSubscriptionEstablishedCallback : SubscriptionEstablishedCallback {
    override fun onSubscriptionEstablished(subscriptionId: Long) {
      this@PairOnNetworkLongImSubscribeCommand.subscriptionId = subscriptionId
      logger.log(Level.INFO, "onSubscriptionEstablished with Id" + subscriptionId)
      setSuccess()
    }
  }

  private inner class InternalResubscriptionAttemptCallback : ResubscriptionAttemptCallback {
    override fun onResubscriptionAttempt(
      terminationCause: Long,
      nextResubscribeIntervalMsec: Long
    ) {
      logger.log(Level.INFO, "ResubscriptionAttemptCallback")
    }
  }

  override fun runCommand() {
    val attributePathList =
      listOf(
        ChipAttributePath.newInstance(
          /* endpointId= */ 0,
          CLUSTER_ID_BASIC,
          ATTR_ID_LOCAL_CONFIG_DISABLED
        )
      )

    val dataVersionFilterList =
      listOf(
        DataVersionFilter.newInstance(
          ChipPathId.forId(/* endpointId= */ 0),
          ChipPathId.forId(CLUSTER_ID_BASIC),
          CLUSTER_ID_BASIC_VERSION,
        )
      )

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
    currentCommissioner()
      .getConnectedDevicePointer(getNodeId(), InternalGetConnectedDeviceCallback())
    clear()
    currentCommissioner()
      .subscribeToPath(
        InternalSubscriptionEstablishedCallback(),
        InternalResubscriptionAttemptCallback(),
        InternalReportCallback(),
        devicePointer,
        attributePathList,
        Collections.emptyList(),
        dataVersionFilterList,
        0,
        5,
        false,
        false,
        0
      )
    waitCompleteMs(getTimeoutMillis())
    currentCommissioner()
      .shutdownSubscriptions(currentCommissioner().getFabricIndex(), getNodeId(), subscriptionId)
  }

  companion object {
    private val logger = Logger.getLogger(PairOnNetworkLongImSubscribeCommand::class.java.name)

    private const val MATTER_PORT = 5540
    private const val CLUSTER_ID_BASIC = 0x0028L
    private const val ATTR_ID_LOCAL_CONFIG_DISABLED = 16L
    private const val CLUSTER_ID_BASIC_VERSION = 0L
  }
}
