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

import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback
import chip.devicecontroller.WriteAttributesCallback
import chip.devicecontroller.model.AttributeWriteRequest
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.Status
import com.matter.controller.commands.common.CredentialsIssuer
import java.util.logging.Level
import java.util.logging.Logger
import matter.tlv.AnonymousTag
import matter.tlv.TlvWriter

class PairOnNetworkLongImWriteCommand(
  controller: ChipDeviceController,
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
  private var devicePointer: Long = 0

  private inner class InternalWriteAttributesCallback : WriteAttributesCallback {
    override fun onError(attributePath: ChipAttributePath?, e: Exception) {
      logger.log(Level.INFO, "Write receive onError on ")
      if (attributePath != null) {
        logger.log(Level.INFO, attributePath.toString())
      }
      setFailure("write failure")
    }

    override fun onResponse(attributePath: ChipAttributePath, status: Status) {
      logger.log(Level.INFO, "$attributePath : Write response: $status")
      setSuccess()
    }
  }

  private inner class InternalGetConnectedDeviceCallback : GetConnectedDeviceCallback {
    override fun onDeviceConnected(devicePointer: Long) {
      this@PairOnNetworkLongImWriteCommand.devicePointer = devicePointer
      logger.log(Level.INFO, "onDeviceConnected")
    }

    override fun onConnectionFailure(nodeId: Long, error: Exception?) {
      logger.log(Level.INFO, "onConnectionFailure")
    }
  }

  override fun runCommand() {
    val tlvWriter1 = TlvWriter()
    tlvWriter1.put(AnonymousTag, true)
    val attributeList1 =
      listOf(
        AttributeWriteRequest.newInstance(
          /* endpointId= */ 0,
          CLUSTER_ID_BASIC,
          ATTR_ID_LOCAL_CONFIG_DISABLED,
          tlvWriter1.getEncoded()
        )
      )

    val attributeList2 =
      listOf(
        AttributeWriteRequest.newInstance(
          /* endpointId= */ 0,
          CLUSTER_ID_BASIC,
          ATTR_ID_LOCAL_CONFIG_DISABLED,
          """{"40:BOOL":false}"""
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
      .write(InternalWriteAttributesCallback(), devicePointer, attributeList1, 0, 0)
    waitCompleteMs(getTimeoutMillis())
    clear()
    currentCommissioner()
      .write(InternalWriteAttributesCallback(), devicePointer, attributeList2, 0, 0)
    waitCompleteMs(getTimeoutMillis())
  }

  companion object {
    private val logger = Logger.getLogger(PairOnNetworkLongImWriteCommand::class.java.name)

    private const val MATTER_PORT = 5540
    private const val CLUSTER_ID_BASIC = 0x0028L
    private const val ATTR_ID_LOCAL_CONFIG_DISABLED = 16L
  }
}
