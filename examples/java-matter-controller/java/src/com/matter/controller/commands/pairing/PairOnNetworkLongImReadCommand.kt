package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.model.AttributeState
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.ChipPathId
import chip.devicecontroller.model.EventState
import chip.devicecontroller.model.NodeState
import com.matter.controller.commands.common.CredentialsIssuer
import java.util.logging.Level
import java.util.logging.Logger

class PairOnNetworkLongImReadCommand(
  controller: ChipDeviceController,
  credsIssue: CredentialsIssuer?
) :
  PairingCommand(
    controller,
    "onnetwork-long-im-read",
    credsIssue,
    PairingModeType.ON_NETWORK,
    PairingNetworkType.NONE,
    DiscoveryFilterType.LONG_DISCRIMINATOR
  ) {
  private var devicePointer: Long = 0

  private inner class InternalReportCallback : ReportCallback {
    override fun onError(
      attributePath: ChipAttributePath?,
      eventPath: ChipEventPath?,
      e: Exception
    ) {
      logger.log(Level.INFO, "Read receive onError")
      setFailure("read failure")
    }

    // kotlin-detect complains that bytearray as a magic number, but we cannot define bytearray
    // as a well named constant and const can only support with primitive and string.
    @Suppress("MagicNumber")
    fun checkLocalConfigDisableAttributeTlv(attribute: AttributeState): Boolean =
      attribute.getTlv().contentEquals(byteArrayOf(0x8))

    fun checkLocalConfigDisableAttributeJson(attribute: AttributeState): Boolean =
      attribute.getJson().toString() == """{"16:BOOL":false}"""

    // kotlin-detect complains that bytearray as a magic number, but we cannot define bytearray
    // as a well named constant and const can only support with primitive and string.
    @Suppress("MagicNumber")
    fun checkStartUpEventTlv(event: EventState): Boolean =
      event.getTlv().contentEquals(byteArrayOf(0x15, 0x24, 0x0, 0x1, 0x18))

    fun checkStartUpEventJson(event: EventState): Boolean =
      event.getJson().toString() == """{"0:STRUCT":{"0:UINT":1}}"""

    fun checkAllAttributesJsonForBasicCluster(cluster: String): Boolean {
      val expected =
        """{"16:BOOL":false,""" +
          """"65531:ARRAY-UINT":[""" +
          """0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,18,19,20,65528,65529,65531,65532,65533]}"""
      return cluster.equals(expected)
    }

    private fun validateResponse(nodeState: NodeState) {
      val endpointZero =
        requireNotNull(nodeState.getEndpointState(0)) { "Endpoint zero not found." }

      val basicCluster =
        requireNotNull(endpointZero.getClusterState(CLUSTER_ID_BASIC)) {
          "Basic cluster not found."
        }

      val localConfigDisabledAttribute =
        requireNotNull(basicCluster.getAttributeState(ATTR_ID_LOCAL_CONFIG_DISABLED)) {
          "No local config disabled attribute found."
        }

      val startUpEvents =
        requireNotNull(basicCluster.getEventState(EVENT_ID_START_UP)) { "No start up event found." }

      val clusterAttributes =
        requireNotNull(basicCluster.getAttributesJson()) { "No basicCluster attribute found." }

      require(checkLocalConfigDisableAttributeTlv(localConfigDisabledAttribute)) {
        "Invalid local config disabled attribute TLV ${localConfigDisabledAttribute.getTlv().contentToString()}"
      }

      require(checkLocalConfigDisableAttributeJson(localConfigDisabledAttribute)) {
        "Invalid local config disabled attribute Json ${localConfigDisabledAttribute.getJson().toString()}"
      }

      require(startUpEvents.isNotEmpty()) { "Unexpected: startUpEvents is empty" }

      require(checkStartUpEventTlv(startUpEvents[0])) {
        "Invalid start up event TLV ${startUpEvents[0].getTlv().contentToString()}"
      }

      require(checkStartUpEventJson(startUpEvents[0])) {
        "Invalid start up event Json ${startUpEvents[0].getJson().toString()}"
      }

      require(checkAllAttributesJsonForBasicCluster(clusterAttributes)) {
        "Invalid basic cluster attributes Json ${clusterAttributes}"
      }
    }

    override fun onReport(nodeState: NodeState) {
      logger.log(Level.INFO, nodeState.toString())
      try {
        validateResponse(nodeState)
        setSuccess()
      } catch (ex: IllegalArgumentException) {
        setFailure(ex.message)
      }
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
    val attributePathList =
      listOf(
        ChipAttributePath.newInstance(
          ChipPathId.forId(/* endpointId= */ 0),
          ChipPathId.forId(CLUSTER_ID_BASIC),
          ChipPathId.forId(ATTR_ID_LOCAL_CONFIG_DISABLED),
        ),
        ChipAttributePath.newInstance(
          ChipPathId.forId(/* endpointId= */ 0),
          ChipPathId.forId(CLUSTER_ID_BASIC),
          ChipPathId.forId(GLOBAL_ATTRIBUTE_LIST),
        )
      )

    val eventPathList =
      listOf(
        ChipEventPath.newInstance(
          ChipPathId.forWildcard(),
          ChipPathId.forWildcard(),
          ChipPathId.forWildcard()
        )
      )

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
      .readPath(InternalReportCallback(), devicePointer, attributePathList, eventPathList, false, 0)
    waitCompleteMs(getTimeoutMillis())
  }

  companion object {
    private val logger = Logger.getLogger(PairOnNetworkLongImReadCommand::class.java.name)

    private const val MATTER_PORT = 5540
    private const val CLUSTER_ID_BASIC = 0x0028L
    private const val ATTR_ID_LOCAL_CONFIG_DISABLED = 16L
    private const val EVENT_ID_START_UP = 0L
    private const val GLOBAL_ATTRIBUTE_LIST = 65531L
  }
}
