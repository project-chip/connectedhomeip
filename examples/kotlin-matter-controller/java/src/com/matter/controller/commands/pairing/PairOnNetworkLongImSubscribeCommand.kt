package com.matter.controller.commands.pairing

import com.matter.controller.commands.common.CredentialsIssuer
import java.util.logging.Level
import java.util.logging.Logger
import kotlinx.coroutines.flow.takeWhile
import kotlinx.coroutines.runBlocking
import matter.controller.MatterController
import matter.controller.SubscribeRequest
import matter.controller.SubscriptionState
import matter.controller.model.AttributePath
import matter.controller.model.EventPath

class PairOnNetworkLongImSubscribeCommand(
  controller: MatterController,
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
  override fun runCommand() {
    val attributePaths =
      listOf(
        AttributePath(
          endpointId = WILDCARD_ENDPOINT_ID,
          clusterId = WILDCARD_CLUSTER_ID,
          attributeId = WILDCARD_EVENT_ID,
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

    val subscribeRequest: SubscribeRequest = SubscribeRequest(eventPaths, attributePaths)

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
        startSubscription(subscribeRequest)
      } catch (ex: Exception) {
        logger.log(Level.WARNING, "General subscribe failure occurred with error ${ex.message}")
        setFailure("subscribe failure")
      } finally {
        clear()
      }
    }

    setSuccess()
  }

  private suspend fun startSubscription(request: SubscribeRequest) {
    logger.log(Level.INFO, "Starting subscription")

    currentCommissioner()
      .subscribe(request)
      .takeWhile { subscriptionState ->
        // Keep collecting as long as it's not SubscriptionEstablished
        subscriptionState !is SubscriptionState.SubscriptionEstablished
      }
      .collect { subscriptionState ->
        when (subscriptionState) {
          is SubscriptionState.NodeStateUpdate -> {
            logger.log(Level.INFO, "Received NodeStateUpdate: ${subscriptionState.updateState}")

            // TODO: Add more validation rules as needed
          }
          is SubscriptionState.SubscriptionErrorNotification -> {
            logger.log(
              Level.WARNING,
              "Received SubscriptionErrorNotification with terminationCause: ${subscriptionState.terminationCause}"
            )
          }
          is SubscriptionState.SubscriptionEstablished -> {
            logger.log(Level.INFO, "Subscription is established")
          }
          else -> {
            logger.log(Level.SEVERE, "Unexpected subscription state: $subscriptionState")
          }
        }
      }
  }

  companion object {
    private val logger = Logger.getLogger(PairOnNetworkLongImSubscribeCommand::class.java.name)

    private const val MATTER_PORT = 5540
    private const val WILDCARD_ENDPOINT_ID: UShort = 0xffffu
    private const val WILDCARD_CLUSTER_ID: UInt = 0xffffffffu
    private const val WILDCARD_ATTRIBUTE_ID: UInt = 0xffffffffu
    private const val WILDCARD_EVENT_ID: UInt = 0xffffffffu
  }
}
