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

import chip.devicecontroller.ICDDeviceInfo
import com.matter.controller.commands.common.CredentialsIssuer
import com.matter.controller.commands.common.IPAddress
import com.matter.controller.commands.common.MatterCommand
import java.net.InetAddress
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.atomic.AtomicLong
import java.util.logging.Level
import java.util.logging.Logger
import matter.controller.MatterController

abstract class PairingCommand(
  controller: MatterController,
  commandName: String,
  credsIssuer: CredentialsIssuer?,
  private val pairingMode: PairingModeType = PairingModeType.NONE,
  private val networkType: PairingNetworkType = PairingNetworkType.NONE,
  private val filterType: DiscoveryFilterType = DiscoveryFilterType.NONE
) : MatterCommand(controller, credsIssuer, commandName), MatterController.CompletionListener {
  private val remoteAddr: IPAddress = IPAddress(InetAddress.getByName("::1"))
  private val nodeId = AtomicLong()
  private val discoveryFilterCode = AtomicLong()
  private val timeoutMillis = AtomicLong()
  private val discoverOnce = AtomicBoolean()
  private val useOnlyOnNetworkDiscovery = AtomicBoolean()
  private val remotePort = AtomicInteger()
  private val discriminator = AtomicInteger()
  private val setupPINCode = AtomicLong()
  private val operationalDataset = StringBuffer()
  private val ssid = StringBuffer()
  private val password = StringBuffer()
  private val onboardingPayload = StringBuffer()
  private val discoveryFilterInstanceName = StringBuffer()

  init {
    addArgument("node-id", 0, Long.MAX_VALUE, nodeId, null, false)

    when (networkType) {
      PairingNetworkType.NONE -> {}
      PairingNetworkType.WIFI -> {
        addArgument("ssid", ssid, null, false)
        addArgument("password", password, null, false)
      }
      PairingNetworkType.THREAD ->
        addArgument("operationalDataset", operationalDataset, null, false)
    }

    when (pairingMode) {
      PairingModeType.NONE -> {}
      PairingModeType.CODE,
      PairingModeType.CODE_PASE_ONLY -> {
        addArgument("payload", onboardingPayload, null, false)
        addArgument("discover-once", discoverOnce, null, true)
        addArgument("use-only-onnetwork-discovery", useOnlyOnNetworkDiscovery, null, true)
      }
      PairingModeType.ADDRESS_PASE_ONLY -> {
        addArgument("setup-pin-code", 0, 134217727, setupPINCode, null, false)
        addArgument("device-remote-ip", remoteAddr, false)
        addArgument("device-remote-port", 0.toShort(), Short.MAX_VALUE, remotePort, null, false)
      }
      PairingModeType.BLE -> {
        addArgument("setup-pin-code", 0, 134217727, setupPINCode, null, false)
        addArgument("discriminator", 0.toShort(), 4096.toShort(), discriminator, null, false)
      }
      PairingModeType.ON_NETWORK ->
        addArgument("setup-pin-code", 0, 134217727, setupPINCode, null, false)
      PairingModeType.SOFT_AP -> {
        addArgument("setup-pin-code", 0, 134217727, setupPINCode, null, false)
        addArgument("discriminator", 0.toShort(), 4096.toShort(), discriminator, null, false)
        addArgument("device-remote-ip", remoteAddr, false)
        addArgument("device-remote-port", 0.toShort(), Short.MAX_VALUE, remotePort, null, false)
      }
      PairingModeType.ALREADY_DISCOVERED -> {
        addArgument("setup-pin-code", 0, 134217727, setupPINCode, null, false)
        addArgument("device-remote-ip", remoteAddr, false)
        addArgument("device-remote-port", 0.toShort(), Short.MAX_VALUE, remotePort, null, false)
      }
    }

    when (filterType) {
      DiscoveryFilterType.NONE -> {}
      DiscoveryFilterType.SHORT_DISCRIMINATOR,
      DiscoveryFilterType.LONG_DISCRIMINATOR ->
        addArgument("discriminator", 0.toShort(), 4096.toShort(), discriminator, null, false)
      DiscoveryFilterType.VENDOR_ID ->
        addArgument("vendor-id", 1.toShort(), Short.MAX_VALUE, discoveryFilterCode, null, false)
      DiscoveryFilterType.COMPRESSED_FABRIC_ID ->
        addArgument("fabric-id", 0L, Long.MAX_VALUE, discoveryFilterCode, null, false)
      DiscoveryFilterType.COMMISSIONING_MODE,
      DiscoveryFilterType.COMMISSIONER -> {}
      DiscoveryFilterType.DEVICE_TYPE ->
        addArgument("device-type", 0.toShort(), Short.MAX_VALUE, discoveryFilterCode, null, false)
      DiscoveryFilterType.INSTANCE_NAME ->
        addArgument("name", discoveryFilterInstanceName, null, false)
    }

    addArgument("timeout", 0L, Long.MAX_VALUE, timeoutMillis, null, false)
  }

  override fun onConnectDeviceComplete() {
    logger.log(Level.INFO, "onConnectDeviceComplete")
  }

  override fun onStatusUpdate(status: Int) {
    logger.log(Level.INFO, "onStatusUpdate with status: $status")
  }

  override fun onPairingComplete(errorCode: UInt) {
    logger.log(Level.INFO, "onPairingComplete with error code: $errorCode")
    if (errorCode != 0U) {
      setFailure("onPairingComplete failure")
    }
  }

  override fun onPairingDeleted(errorCode: UInt) {
    logger.log(Level.INFO, "onPairingDeleted with error code: $errorCode")
  }

  override fun onCommissioningComplete(nodeId: Long, errorCode: UInt) {
    logger.log(Level.INFO, "onCommissioningComplete with error code: $errorCode")
    if (errorCode == 0U) {
      setSuccess()
    } else {
      setFailure("onCommissioningComplete failure")
    }
  }

  override fun onReadCommissioningInfo(
    vendorId: Int,
    productId: Int,
    wifiEndpointId: Int,
    threadEndpointId: Int
  ) {
    logger.log(Level.INFO, "onReadCommissioningInfo")
  }

  override fun onCommissioningStatusUpdate(nodeId: Long, stage: String?, errorCode: UInt) {
    logger.log(Level.INFO, "onCommissioningStatusUpdate")
  }

  override fun onNotifyChipConnectionClosed() {
    logger.log(Level.INFO, "onNotifyChipConnectionClosed")
  }

  override fun onError(error: Throwable) {
    setFailure(error.toString())
    logger.log(Level.INFO, "onError with error: $error")
  }

  override fun onOpCSRGenerationComplete(csr: ByteArray) {
    logger.log(Level.INFO, "onOpCSRGenerationComplete")
    for (i in csr.indices) {
      print(csr[i].toString() + " ")
    }
  }

  override fun onICDRegistrationInfoRequired() {
    logger.log(Level.INFO, "onICDRegistrationInfoRequired")
  }

  override fun onICDRegistrationComplete(errorCode: UInt, icdDeviceInfo: ICDDeviceInfo) {
    logger.log(
      Level.INFO,
      "onICDRegistrationComplete with errorCode: $errorCode, symmetricKey: ${icdDeviceInfo.symmetricKey.toHex()}, icdDeviceInfo: $icdDeviceInfo"
    )
  }

  fun getNodeId(): Long {
    return nodeId.get()
  }

  fun getRemoteAddr(): IPAddress {
    return remoteAddr
  }

  fun getRemotePort(): Int {
    return remotePort.get()
  }

  fun getSetupPINCode(): Long {
    return setupPINCode.get()
  }

  fun getDiscriminator(): Int {
    return discriminator.get()
  }

  fun getTimeoutMillis(): Long {
    return timeoutMillis.get()
  }

  fun getOnboardingPayload(): String {
    return onboardingPayload.toString()
  }

  private fun ByteArray.toHex(): String =
    joinToString(separator = "") { eachByte -> "%02x".format(eachByte) }

  private fun String.hexToByteArray(): ByteArray {
    return chunked(2).map { byteStr -> byteStr.toUByte(16).toByte() }.toByteArray()
  }

  fun getDiscoverOnce(): Boolean {
    return discoverOnce.get()
  }

  fun getUseOnlyOnNetworkDiscovery(): Boolean {
    return useOnlyOnNetworkDiscovery.get()
  }

  companion object {
    private val logger = Logger.getLogger(PairingCommand::class.java.name)
  }
}
