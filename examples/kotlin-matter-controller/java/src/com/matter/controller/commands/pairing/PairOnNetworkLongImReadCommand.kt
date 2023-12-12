package com.matter.controller.commands.pairing

import com.matter.controller.commands.common.CredentialsIssuer
import java.util.logging.Level
import java.util.logging.Logger
import kotlinx.coroutines.runBlocking
import matter.controller.MatterController
import matter.controller.ReadData
import matter.controller.ReadRequest
import matter.controller.ReadResponse
import matter.controller.model.AttributePath
import matter.controller.model.EventPath

class PairOnNetworkLongImReadCommand(controller: MatterController, credsIssue: CredentialsIssuer?) :
  PairingCommand(
    controller,
    "onnetwork-long-im-read",
    credsIssue,
    PairingModeType.ON_NETWORK,
    PairingNetworkType.NONE,
    DiscoveryFilterType.LONG_DISCRIMINATOR
  ) {
  override fun runCommand() {
    val attributePaths =
      listOf(
        AttributePath(
          endpointId = DEFAULT_ENDPOINT,
          clusterId = CLUSTER_ID_BASIC,
          attributeId = ATTR_ID_LOCAL_CONFIG_DISABLED,
        ),
        AttributePath(
          endpointId = DEFAULT_ENDPOINT,
          clusterId = CLUSTER_ID_BASIC,
          attributeId = GLOBAL_ATTRIBUTE_LIST,
        )
      )

    val eventPaths =
      listOf(
        EventPath(
          endpointId = WILDCARD_ENDPOINT_ID,
          clusterId = WILDCARD_CLUSTER_ID,
          eventId = WILDCARD_EVENT_ID
        )
      )

    val readRequest: ReadRequest = ReadRequest(eventPaths, attributePaths)

    currentCommissioner()
      .pairDevice(
        getNodeId(),
        getRemoteAddr().address.hostAddress,
        MATTER_PORT,
        getDiscriminator(),
        getSetupPINCode(),
      )
    currentCommissioner().setCompletionListener(this)
    waitCompleteMs(getTimeoutMillis())

    runBlocking {
      try {
        val response: ReadResponse = currentCommissioner().read(readRequest)
        logger.log(Level.INFO, "Read command succeeded")
        validateResponse(response)
      } catch (ex: Exception) {
        logger.log(Level.WARNING, "General read failure occurred with error ${ex.message}")
        setFailure("read failure")
      } finally {
        clear()
      }
    }

    setSuccess()
  }

  private fun findAttributeById(successes: List<ReadData>, id: UInt): ReadData.Attribute? {
    return successes.filterIsInstance<ReadData.Attribute>().find { it.path.attributeId == id }
  }

  private fun findEventById(successes: List<ReadData>, id: UInt): ReadData.Event? {
    return successes.filterIsInstance<ReadData.Event>().find { it.path.eventId == id }
  }

  private fun validateResponse(response: ReadResponse) {
    require(response.successes.isNotEmpty()) { "Unexpected: response.successes is empty" }
    require(response.failures.isEmpty()) { "Unexpected: response.failures is not empty" }

    val localConfigDisabledAttribute =
      findAttributeById(response.successes, ATTR_ID_LOCAL_CONFIG_DISABLED)
    requireNotNull(localConfigDisabledAttribute) { "No local config disabled attribute found." }

    // TODO: Add more validation rules as needed
  }

  companion object {
    private val logger = Logger.getLogger(PairOnNetworkLongImReadCommand::class.java.name)

    private const val MATTER_PORT = 5540
    private const val DEFAULT_ENDPOINT: UShort = 0u
    private const val WILDCARD_ENDPOINT_ID: UShort = 0xffffu
    private const val WILDCARD_CLUSTER_ID: UInt = 0xffffffffu
    private const val WILDCARD_ATTRIBUTE_ID: UInt = 0xffffffffu
    private const val WILDCARD_EVENT_ID: UInt = 0xffffffffu
    private const val CLUSTER_ID_BASIC: UInt = 0x0028u
    private const val ATTR_ID_LOCAL_CONFIG_DISABLED: UInt = 16u
    private const val EVENT_ID_START_UP: UInt = 0u
    private const val GLOBAL_ATTRIBUTE_LIST: UInt = 65531u
  }
}
