package com.matter.controller.commands.pairing

import com.matter.controller.commands.common.CredentialsIssuer
import java.util.logging.Level
import java.util.logging.Logger
import kotlinx.coroutines.runBlocking
import matter.controller.MatterController
import matter.controller.cluster.clusters.BasicInformationCluster

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
    runBlocking {
      try {
        val basicInformationCluster =
          BasicInformationCluster(controller = currentCommissioner(), endpointId = DEFAULT_ENDPOINT)
        val vendorName = basicInformationCluster.readVendorNameAttribute()

        // By running command readVendorIDAttribute, we are implicitly requesting CASE to be
        // established if it's not already present.
        val vendorId = basicInformationCluster.readVendorIDAttribute()
        logger.log(Level.INFO, "Read command succeeded, Verdor Name:${vendorName} (ID:${vendorId})")
      } catch (ex: Exception) {
        logger.log(Level.WARNING, "General read failure occurred with error ${ex.message}")
        setFailure("read failure")
      } finally {
        clear()
      }
    }

    setSuccess()
  }

  companion object {
    private val logger = Logger.getLogger(PairOnNetworkLongImReadCommand::class.java.name)
    private const val MATTER_PORT = 5540
    private const val DEFAULT_ENDPOINT: UShort = 0u
  }
}
