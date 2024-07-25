/*
 *   Copyright (c) 2024 Project CHIP Authors
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
import chip.devicecontroller.ExtendableInvokeCallback
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback
import chip.devicecontroller.model.InvokeElement
import chip.devicecontroller.model.InvokeResponseData
import chip.devicecontroller.model.NoInvokeResponseData
import chip.devicecontroller.model.Status
import com.matter.controller.commands.common.CredentialsIssuer
import java.util.logging.Level
import java.util.logging.Logger
import kotlin.UShort
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvWriter

class PairOnNetworkLongImExtendableInvokeCommand(
  controller: ChipDeviceController,
  credsIssue: CredentialsIssuer?
) :
  PairingCommand(
    controller,
    "onnetwork-long-im-extendable-invoke",
    credsIssue,
    PairingModeType.ON_NETWORK,
    PairingNetworkType.NONE,
    DiscoveryFilterType.LONG_DISCRIMINATOR
  ) {
  private var devicePointer: Long = 0

  private fun setDevicePointer(devicePointer: Long) {
    this.devicePointer = devicePointer
  }

  private inner class InternalInvokeCallback : ExtendableInvokeCallback {
    private var responseCount = 0

    override fun onError(e: Exception) {
      logger.log(Level.INFO, "Batch Invoke receive onError" + e.message)
      setFailure("invoke failure")
    }

    override fun onResponse(invokeResponseData: InvokeResponseData) {
      logger.log(Level.INFO, "Batch Invoke receive OnResponse on $invokeResponseData")
      val clusterId = invokeResponseData.getClusterId().getId()
      val commandId = invokeResponseData.getCommandId().getId()
      val tlvData = invokeResponseData.getTlvByteArray()
      val jsonData = invokeResponseData.getJsonString()
      val status = invokeResponseData.getStatus()

      if (clusterId == CLUSTER_ID_IDENTIFY && commandId == IDENTIFY_COMMAND) {
        if (tlvData != null || jsonData != null) {
          setFailure("invoke failure with problematic payload")
        }
        if (
          status != null && status.status != Status.Code.Success && status.clusterStatus.isPresent()
        ) {
          setFailure("invoke failure with incorrect status")
        }
      }

      if (clusterId == CLUSTER_ID_TEST && commandId == TEST_ADD_ARGUMENT_RSP_COMMAND) {
        if (tlvData == null || jsonData == null) {
          setFailure("invoke failure with problematic payload")
        }

        if (!jsonData.equals("""{"0:UINT":2}""")) {
          setFailure("invoke failure with problematic json")
        }

        if (status != null) {
          setFailure("invoke failure with incorrect status")
        }
      }
      responseCount++
    }

    override fun onNoResponse(noInvokeResponseData: NoInvokeResponseData) {
      logger.log(Level.INFO, "Batch Invoke receive onNoResponse on $noInvokeResponseData")
    }

    override fun onDone() {
      if (responseCount == TEST_COMMONDS_NUM) {
        setSuccess()
      } else {
        setFailure("invoke failure")
      }
    }
  }

  private inner class InternalGetConnectedDeviceCallback : GetConnectedDeviceCallback {
    override fun onDeviceConnected(devicePointer: Long) {
      setDevicePointer(devicePointer)
      logger.log(Level.INFO, "onDeviceConnected")
    }

    override fun onConnectionFailure(nodeId: Long, error: Exception) {
      logger.log(Level.INFO, "onConnectionFailure")
    }
  }

  override fun runCommand() {
    val number: UShort = 1u
    val tlvWriter1 = TlvWriter()
    tlvWriter1.startStructure(AnonymousTag)
    tlvWriter1.put(ContextSpecificTag(0), number)
    tlvWriter1.endStructure()

    val element1: InvokeElement =
      InvokeElement.newInstance(
        /* endpointId= */ 0,
        CLUSTER_ID_IDENTIFY,
        IDENTIFY_COMMAND,
        tlvWriter1.getEncoded(),
        null
      )

    val tlvWriter2 = TlvWriter()
    tlvWriter2.startStructure(AnonymousTag)
    tlvWriter2.put(ContextSpecificTag(0), number)
    tlvWriter2.put(ContextSpecificTag(1), number)
    tlvWriter2.endStructure()

    val element2: InvokeElement =
      InvokeElement.newInstance(
        /* endpointId= */ 1,
        CLUSTER_ID_TEST,
        TEST_ADD_ARGUMENT_COMMAND,
        tlvWriter2.getEncoded(),
        null
      )

    val invokeList = listOf(element1, element2)
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
      .extendableInvoke(InternalInvokeCallback(), devicePointer, invokeList, 0, 0)
    waitCompleteMs(getTimeoutMillis())
  }

  companion object {
    private val logger =
      Logger.getLogger(PairOnNetworkLongImExtendableInvokeCommand::class.java.name)

    private const val MATTER_PORT = 5540
    private const val CLUSTER_ID_IDENTIFY = 0x0003L
    private const val IDENTIFY_COMMAND = 0L
    private const val CLUSTER_ID_TEST = 0xFFF1FC05L
    private const val TEST_ADD_ARGUMENT_COMMAND = 0X04L
    private const val TEST_ADD_ARGUMENT_RSP_COMMAND = 0X01L
    private const val TEST_COMMONDS_NUM = 2
  }
}
