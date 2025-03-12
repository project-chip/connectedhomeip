package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.model.AttributeState
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.ChipPathId
import chip.devicecontroller.model.DataVersionFilter
import chip.devicecontroller.model.EventState
import chip.devicecontroller.model.NodeState
import chip.devicecontroller.model.Status
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

    fun checkAllAttributesJsonForFixedLabel(cluster: String): Boolean {
      // TODO: this hard-codes the array and as a result it is order-dependend. This should
      //       be changed to be order-independent.
      val expected =
        """{"65528:ARRAY-?":[],"0:ARRAY-STRUCT":[{"0:STRING":"room","1:STRING":"bedroom 2"},""" +
          """{"0:STRING":"orientation","1:STRING":"North"},{"0:STRING":"floor","1:STRING":"2"},""" +
          """{"0:STRING":"direction","1:STRING":"up"}],"65531:ARRAY-UINT":[0,65532,65533,65528,65529,65531],""" +
          """"65533:UINT":1,"65529:ARRAY-?":[],"65532:UINT":0}"""
      return cluster.equals(expected)
    }

    fun checkUnitTestClusterGeneralStatus(status: Status): Boolean =
      (status.getStatus() == Status.Code.InvalidDataType) && !status.getClusterStatus().isPresent()

    fun checkUnitTestClusterClusterStatus(status: Status): Boolean =
      (status.getStatus() == Status.Code.Failure) &&
        status.getClusterStatus().isPresent() &&
        (status.getClusterStatus().get() == CLUSTER_ID_TEST_CLUSTER_ERROR_CLUSTER_STATUS)

    private fun validateResponse(nodeState: NodeState) {
      val endpointZero =
        requireNotNull(nodeState.getEndpointState(0)) { "Endpoint zero not found." }

      val endpointOne = requireNotNull(nodeState.getEndpointState(1)) { "Endpoint one not found." }

      val basicCluster =
        requireNotNull(endpointZero.getClusterState(CLUSTER_ID_BASIC)) {
          "Basic cluster not found."
        }

      val fixedLabelCluster =
        requireNotNull(endpointOne.getClusterState(FIXED_LABEL_CLUSTER)) {
          "fixed label cluster not found."
        }

      val localConfigDisabledAttribute =
        requireNotNull(basicCluster.getAttributeState(ATTR_ID_LOCAL_CONFIG_DISABLED)) {
          "No local config disabled attribute found."
        }

      val unitTestCluster =
        requireNotNull(endpointOne.getClusterState(UNIT_TEST_CLUSTER)) {
          "Unit test cluster not found."
        }

      val startUpEvents =
        requireNotNull(basicCluster.getEventState(EVENT_ID_START_UP)) { "No start up event found." }

      val clusterAttributes =
        requireNotNull(fixedLabelCluster.getAttributesJson()) {
          "No fixed label cluster attribute found."
        }

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

      require(checkAllAttributesJsonForFixedLabel(clusterAttributes)) {
        "Invalid fixed label cluster attributes Json ${clusterAttributes}"
      }

      require(
        checkUnitTestClusterGeneralStatus(
          unitTestCluster.getAttributeStatuses()[CLUSTER_ID_TEST_GENERAL_ERROR_BOOLEAN]!!
        )
      ) {
        "Invalid unit test cluster generalStatus check ${unitTestCluster}"
      }

      require(
        checkUnitTestClusterClusterStatus(
          unitTestCluster.getAttributeStatuses()[CLUSTER_ID_TEST_CLUSTER_ERROR_BOOLEAN]!!
        )
      ) {
        "Invalid unit test cluster clusterStatus check ${unitTestCluster}"
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
          ChipPathId.forWildcard(),
          ChipPathId.forWildcard(),
          ChipPathId.forWildcard()
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
      .readPath(
        InternalReportCallback(),
        devicePointer,
        attributePathList,
        eventPathList,
        dataVersionFilterList,
        false,
        0
      )
    waitCompleteMs(getTimeoutMillis())
  }

  companion object {
    private val logger = Logger.getLogger(PairOnNetworkLongImReadCommand::class.java.name)

    private const val MATTER_PORT = 5540
    private const val CLUSTER_ID_BASIC = 0x0028L
    private const val FIXED_LABEL_CLUSTER = 0x0040L
    private const val UNIT_TEST_CLUSTER = 0xfff1fc05L
    private const val ATTR_ID_LOCAL_CONFIG_DISABLED = 16L
    private const val EVENT_ID_START_UP = 0L
    private const val GLOBAL_ATTRIBUTE_LIST = 65531L
    private const val CLUSTER_ID_BASIC_VERSION = 0L
    private const val CLUSTER_ID_TEST_GENERAL_ERROR_BOOLEAN = 0x0031L
    private const val CLUSTER_ID_TEST_CLUSTER_ERROR_BOOLEAN = 0x0032L
    private const val CLUSTER_ID_TEST_CLUSTER_ERROR_CLUSTER_STATUS = 17
  }
}
