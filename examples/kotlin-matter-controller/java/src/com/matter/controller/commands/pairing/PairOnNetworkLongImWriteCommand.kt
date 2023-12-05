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
import matter.controller.MatterController
import matter.controller.WriteRequest
import matter.controller.WriteRequests
import matter.controller.WriteResponse
import matter.controller.model.AttributePath
import matter.tlv.AnonymousTag
import matter.tlv.TlvWriter

class PairOnNetworkLongImWriteCommand(
  controller: MatterController,
  credsIssue: CredentialsIssuer?
) :
  PairingCommand(
    controller,
    "onnetwork-long-im-write",
    credsIssue,
    PairingModeType.ON_NETWORK,
    PairingNetworkType.NONE,
    DiscoveryFilterType.LONG_DISCRIMINATOR
  ) {
  override fun runCommand() {
    val tlvWriter1 = TlvWriter()
    tlvWriter1.put(AnonymousTag, true)
    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId = 0u,
                  clusterId = CLUSTER_ID_BASIC,
                  attributeId = ATTR_ID_LOCAL_CONFIG_DISABLED
                ),
              tlvPayload = tlvWriter1.getEncoded()
            )
          ),
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
        val response: WriteResponse = currentCommissioner().write(writeRequests)

        if (response is WriteResponse.Success) {
          logger.log(Level.INFO, "Write command succeeded")
        } else if (response is WriteResponse.PartialWriteFailure) {
          logger.log(
            Level.WARNING,
            "Partial write failure occurred with ${response.failures.size} errors"
          )

          for ((index, error) in response.failures.withIndex()) {
            logger.log(Level.WARNING, "Error ${index + 1}:")
            logger.log(Level.WARNING, "Attribute Path: ${error.attributePath}")
            logger.log(Level.WARNING, "Exception Message: ${error.ex.message}")
          }

          setFailure("invoke failure")
        }
      } catch (ex: Exception) {
        setFailure("invoke failure: ${ex.message}")
      } catch (ex: Exception) {
        logger.log(Level.WARNING, "General write failure occurred with error ${ex.message}")
        setFailure("invoke failure")
      } finally {
        clear()
      }
    }

    setSuccess()
  }

  companion object {
    private val logger = Logger.getLogger(PairOnNetworkLongImWriteCommand::class.java.name)

    private const val MATTER_PORT = 5540
    private const val CLUSTER_ID_BASIC = 0x0028u
    private const val ATTR_ID_LOCAL_CONFIG_DISABLED = 16u
  }
}
