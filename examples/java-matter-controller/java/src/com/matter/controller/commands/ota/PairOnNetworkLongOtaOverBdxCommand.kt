/*
 *   Copyright (c) 2025 Project CHIP Authors
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

package com.matter.controller.commands.ota

import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback
import chip.devicecontroller.InvokeCallback
import chip.devicecontroller.OTAProviderDelegate
import chip.devicecontroller.model.InvokeElement
import com.matter.controller.commands.common.CredentialsIssuer
import com.matter.controller.commands.pairing.DiscoveryFilterType
import com.matter.controller.commands.pairing.PairingCommand
import com.matter.controller.commands.pairing.PairingModeType
import com.matter.controller.commands.pairing.PairingNetworkType
import java.io.BufferedInputStream
import java.io.File
import java.io.FileInputStream
import java.io.IOException
import java.io.InputStream
import java.util.logging.Level
import java.util.logging.Logger
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvWriter

class PairOnNetworkLongOtaOverBdxCommand(
  val controller: ChipDeviceController,
  credsIssue: CredentialsIssuer?
) :
  PairingCommand(
    controller,
    "onnetwork-long-ota-over-bdx",
    credsIssue,
    PairingModeType.ON_NETWORK,
    PairingNetworkType.NONE,
    DiscoveryFilterType.LONG_DISCRIMINATOR
  ) {
  private var devicePointer: Long = 0
  private val uri: StringBuffer = StringBuffer()
  private val filename: StringBuffer = StringBuffer()

  init {
    addArgument("uri", uri, null, false)
    addArgument("filename", filename, null, false)
  }

  private fun setDevicePointer(devicePointer: Long) {
    this.devicePointer = devicePointer
  }

  private inner class InternalAnnouceOTACallback : InvokeCallback {
    override fun onError(e: Exception) {
      logger.log(Level.INFO, "AnnouceOTA receive onError" + e.message)
      setFailure("invoke failure")
    }

    override fun onResponse(element: InvokeElement?, successCode: Long) {
      logger.log(
        Level.INFO,
        "AnnouceOTA success code is $successCode and start the bdx transfer soon"
      )
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

  private inner class OtaProviderCallback : OTAProviderDelegate {
    private var fileName: String? = null
    private var version: Long = 0
    private var versionString: String? = null
    private var uri: String? = null

    private var inputStream: InputStream? = null
    private var bufferedInputStream: BufferedInputStream? = null

    private var status: OTAProviderDelegate.QueryImageResponseStatusEnum? = null
    private var delayedTime: UInt? = null
    private var userConsentNeeded: Boolean? = null

    fun setOTAFile(version: Long, versionString: String, fileName: String, uri: String?) {
      this.status = OTAProviderDelegate.QueryImageResponseStatusEnum.UpdateAvailable
      this.version = version
      this.versionString = versionString
      this.fileName = fileName
      this.uri = uri
    }

    override fun handleQueryImage(
      vendorId: Int,
      productId: Int,
      softwareVersion: Long,
      hardwareVersion: Int?,
      location: String?,
      requestorCanConsent: Boolean?,
      metadataForProvider: ByteArray?
    ): OTAProviderDelegate.QueryImageResponse? {
      logger.log(
        Level.INFO,
        "handleQueryImage, $vendorId, $productId, $softwareVersion, $hardwareVersion, $location"
      )
      return when (status) {
        OTAProviderDelegate.QueryImageResponseStatusEnum.UpdateAvailable ->
          OTAProviderDelegate.QueryImageResponse(version, versionString, fileName, null)
        OTAProviderDelegate.QueryImageResponseStatusEnum.Busy ->
          OTAProviderDelegate.QueryImageResponse(
            status,
            delayedTime?.toLong() ?: 0,
            userConsentNeeded ?: false
          )
        OTAProviderDelegate.QueryImageResponseStatusEnum.NotAvailable ->
          OTAProviderDelegate.QueryImageResponse(status, userConsentNeeded ?: false)
        else -> null
      }
    }

    override fun handleOTAQueryFailure(error: Int) {
      setFailure("handleOTAQueryFailure, $error")
    }

    override fun handleApplyUpdateRequest(
      nodeId: Long,
      newVersion: Long
    ): OTAProviderDelegate.ApplyUpdateResponse {
      logger.log(Level.INFO, "handleApplyUpdateRequest, $nodeId, $newVersion")
      return OTAProviderDelegate.ApplyUpdateResponse(
        OTAProviderDelegate.ApplyUpdateActionEnum.Proceed,
        APPLY_WAITING_TIME
      )
    }

    override fun handleNotifyUpdateApplied(nodeId: Long) {
      logger.log(Level.INFO, "handleNotifyUpdateApplied, Finish Firmware Update, $nodeId")
    }

    override fun handleBDXTransferSessionBegin(
      nodeId: Long,
      fileDesignator: String?,
      offset: Long
    ) {
      logger.log(Level.INFO, "handleBDXTransferSessionBegin, $nodeId, $fileDesignator, $offset")
      try {
        val file = File(uri + fileName)
        val fileInputStream = FileInputStream(file)
        bufferedInputStream = BufferedInputStream(fileInputStream)
      } catch (e: IOException) {
        logger.log(Level.INFO, "exception", e)
        inputStream?.close()
        bufferedInputStream?.close()
        inputStream = null
        bufferedInputStream = null
        return
      }
    }

    override fun handleBDXTransferSessionEnd(errorCode: Long, nodeId: Long) {
      logger.log(Level.INFO, "handleBDXTransferSessionEnd, $errorCode, $nodeId")
      inputStream?.close()
      bufferedInputStream?.close()
      inputStream = null
      bufferedInputStream = null
      setSuccess()
    }

    override fun handleBDXQuery(
      nodeId: Long,
      blockSize: Int,
      blockIndex: Long,
      bytesToSkip: Long
    ): OTAProviderDelegate.BDXData? {
      // This code is just example code. This code doesn't check blockIndex and bytesToSkip
      // variable.
      logger.log(
        Level.INFO,
        "handleBDXQuery sending.., $nodeId, $blockSize, $blockIndex, $bytesToSkip"
      )
      if (bufferedInputStream == null) {
        return OTAProviderDelegate.BDXData(ByteArray(0), true)
      }
      val packet = ByteArray(blockSize)
      val len = bufferedInputStream!!.read(packet)

      val sendPacket =
        if (len < blockSize) {
          packet.copyOf(len)
        } else if (len < 0) {
          ByteArray(0)
        } else {
          packet.clone()
        }

      val isEOF = len < blockSize

      return OTAProviderDelegate.BDXData(sendPacket, isEOF)
    }
  }

  override fun runCommand() {
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

    val version = 2L
    val versionString = "2.0"

    logger.log(Level.INFO, "sendAnnounceOTAProviderBtnClick : $filename")

    val otaProviderCallback = OtaProviderCallback()
    currentCommissioner().startOTAProvider(otaProviderCallback)
    otaProviderCallback.setOTAFile(
      version,
      versionString,
      this.filename.toString(),
      this.uri.toString()
    )

    currentCommissioner()
      .getConnectedDevicePointer(getNodeId(), InternalGetConnectedDeviceCallback())
    clear()

    val announceReason: Int = 0
    val annouceOTATlvWriter = TlvWriter()
    annouceOTATlvWriter.startStructure(AnonymousTag)
    annouceOTATlvWriter.put(
      ContextSpecificTag(PROVIDER_NODE_ID_TAG),
      controller.getControllerNodeId().toULong()
    )
    annouceOTATlvWriter.put(ContextSpecificTag(PROVIDER_VENDOR_ID_TAG), TEST_VENDOR_ID.toULong())
    annouceOTATlvWriter.put(ContextSpecificTag(ANNOUCEMENT_REASON_TAG), announceReason.toULong())
    annouceOTATlvWriter.put(ContextSpecificTag(ENDPOINT_TAG), OTA_PROVIDER_ENDPOINT_ID.toULong())
    annouceOTATlvWriter.endStructure()

    val annouceOTACommandElement: InvokeElement =
      InvokeElement.newInstance(
        OTA_REQUESTER_ENDPOINT_ID,
        CLUSTER_ID_OTA_Requestor,
        ANNOUCE_OTA_PROVIDER_COMMAND,
        annouceOTATlvWriter.getEncoded(),
        null
      )
    currentCommissioner()
      .invoke(InternalAnnouceOTACallback(), devicePointer, annouceOTACommandElement, 0, 0)

    waitCompleteMs(getTimeoutMillis())
  }

  companion object {
    private val logger = Logger.getLogger(PairOnNetworkLongOtaOverBdxCommand::class.java.name)
    private const val MATTER_PORT = 5540
    private const val CLUSTER_ID_OTA_Requestor = 0X2AL
    private const val ANNOUCE_OTA_PROVIDER_COMMAND = 0X0L
    private const val OTA_PROVIDER_ENDPOINT_ID = 0
    private const val OTA_REQUESTER_ENDPOINT_ID = 0
    private const val APPLY_WAITING_TIME = 10L
    private const val TEST_VENDOR_ID = 0xFFF1L
    private const val PROVIDER_NODE_ID_TAG = 0
    private const val PROVIDER_VENDOR_ID_TAG = 1
    private const val ANNOUCEMENT_REASON_TAG = 2
    private const val ENDPOINT_TAG = 4
  }
}
