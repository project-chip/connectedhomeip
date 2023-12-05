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
import java.time.Duration
import java.util.logging.Level
import java.util.logging.Logger
import kotlinx.coroutines.runBlocking
import matter.controller.InvokeRequest
import matter.controller.InvokeResponse
import matter.controller.MatterController
import matter.controller.model.CommandPath
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvWriter

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
    val IdentifyTime: UShort = 1u
    val tlvWriter1 = TlvWriter()
    tlvWriter1.startStructure(AnonymousTag)
    tlvWriter1.put(ContextSpecificTag(0), IdentifyTime)
    tlvWriter1.endStructure()

    val element1: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId = 0u, clusterId = CLUSTER_ID_IDENTIFY, commandId = IDENTIFY_COMMAND),
        tlvPayload = tlvWriter1.getEncoded(),
        timedRequest = Duration.ZERO
      )

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
        val response: InvokeResponse = currentCommissioner().invoke(element1)
        logger.log(Level.INFO, "Invoke command succeeded")
        if (response.payload.isNotEmpty()) {
          // TODO:Handle TLV data response
        }
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
    private const val CLUSTER_ID_IDENTIFY = 0x0003u
    private const val IDENTIFY_COMMAND = 0u
    private const val CLUSTER_ID_TEST = 0xFFF1FC05u
    private const val TEST_ADD_ARGUMENT_COMMAND = 0X04u
  }
}
