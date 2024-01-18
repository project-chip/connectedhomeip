/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package com.matter.controller.commands.pairing

import com.matter.controller.commands.common.CredentialsIssuer
import java.util.logging.Level
import java.util.logging.Logger
import kotlinx.coroutines.runBlocking
import matter.controller.MatterController
import matter.controller.cluster.clusters.IdentifyCluster

class PairOnNetworkLongImInvokeCommand(
  controller: MatterController,
  credsIssue: CredentialsIssuer?
) :
  PairingCommand(
    controller,
    "onnetwork-long-im-invoke",
    credsIssue,
    PairingModeType.ON_NETWORK,
    PairingNetworkType.NONE,
    DiscoveryFilterType.LONG_DISCRIMINATOR
  ) {
  override fun runCommand() {
    runBlocking {
      try {
        val identifyTime: UShort = 1u
        val identifyCluster = IdentifyCluster(controller = currentCommissioner(), endpointId = 0u)

        // By running command identify, we are implicitly requesting CASE to be established if it's
        // not already present.
        identifyCluster.identify(identifyTime)
        logger.log(Level.INFO, "Invoke command succeeded")
      } catch (ex: Exception) {
        setFailure("invoke failure: ${ex.message}")
      } finally {
        clear()
      }
    }

    setSuccess()
  }

  companion object {
    private val logger = Logger.getLogger(PairOnNetworkLongImInvokeCommand::class.java.name)
    private const val MATTER_PORT = 5540
  }
}
