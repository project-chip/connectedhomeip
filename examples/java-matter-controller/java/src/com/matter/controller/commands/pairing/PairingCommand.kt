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
import com.matter.controller.commands.common.CredentialsIssuer
import com.matter.controller.commands.common.IPAddress
import com.matter.controller.commands.common.MatterCommand
import java.net.InetAddress
import java.net.UnknownHostException
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.atomic.AtomicLong
import java.util.logging.Level
import java.util.logging.Logger

abstract class PairingCommand(
  controller: ChipDeviceController,
  commandName: String?,
  mode: PairingModeType,
  network: PairingNetworkType,
  credsIssuer: CredentialsIssuer?,
  filter: DiscoveryFilterType
) : MatterCommand(controller, commandName, credsIssuer), ChipDeviceController.CompletionListener {
  private val logger = Logger.getLogger(PairingCommand::class.java.name)
  private var pairingMode: PairingModeType = PairingModeType.NONE
  private var networkType: PairingNetworkType = PairingNetworkType.NONE
  private var filterType = DiscoveryFilterType.NONE
  private var remoteAddr: IPAddress? = null
  private val nodeId: AtomicLong = AtomicLong()
  private val discoveryFilterCode: AtomicLong = AtomicLong()
  private val timeoutMillis: AtomicLong = AtomicLong()
  private val discoverOnce: AtomicBoolean = AtomicBoolean()
  private val useOnlyOnNetworkDiscovery: AtomicBoolean = AtomicBoolean()
  private val remotePort: AtomicInteger = AtomicInteger()
  private val discriminator: AtomicInteger = AtomicInteger()
  private val setupPINCode: AtomicLong = AtomicLong()
  private val operationalDataset = StringBuffer()
  private val SSID = StringBuffer()
  private val password = StringBuffer()
  private val onboardingPayload = StringBuffer()
  private val discoveryFilterInstanceName = StringBuffer()

  constructor(
    controller: ChipDeviceController,
    commandName: String?,
    mode: PairingModeType,
    network: PairingNetworkType,
    credsIssuer: CredentialsIssuer?
  ) : this(controller, commandName, mode, network, credsIssuer, DiscoveryFilterType.NONE)

  init {
    pairingMode = mode
    networkType = network
    filterType = filter
    try {
      remoteAddr = IPAddress(InetAddress.getByName("::1"))
    } catch (e: UnknownHostException) {
      throw RuntimeException(e)
    }
    addArgument("node-id", 0, Long.MAX_VALUE, nodeId, null, false)
    when (networkType) {
      PairingNetworkType.NONE -> {}
      PairingNetworkType.WIFI -> {
        addArgument("ssid", SSID, null, false)
        addArgument("password", password, null, false)
      }

      PairingNetworkType.THREAD -> addArgument("operationalDataset", operationalDataset, null, false)
    }
    when (mode) {
      PairingModeType.NONE -> {}
      PairingModeType.CODE, PairingModeType.CODE_PASE_ONLY -> {
        addArgument("payload", onboardingPayload, null, false)
        addArgument("discover-once", discoverOnce, null, false)
        addArgument("use-only-onnetwork-discovery", useOnlyOnNetworkDiscovery, null, false)
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

      PairingModeType.ON_NETWORK -> addArgument("setup-pin-code", 0, 134217727, setupPINCode, null, false)
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
      DiscoveryFilterType.SHORT_DISCRIMINATOR -> addArgument(
        "discriminator",
        0.toShort(),
        4096.toShort(),
        discriminator,
        null,
        false
      )

      DiscoveryFilterType.LONG_DISCRIMINATOR -> addArgument(
        "discriminator",
        0.toShort(),
        4096.toShort(),
        discriminator,
        null,
        false
      )

      DiscoveryFilterType.VENDOR_ID -> addArgument(
        "vendor-id",
        0,
        Long.MAX_VALUE,
        discoveryFilterCode,
        null,
        false
      )

      DiscoveryFilterType.COMPRESSED_FABRIC_ID -> addArgument(
        "fabric-id",
        0,
        Long.MAX_VALUE,
        discoveryFilterCode,
        null,
        false
      )

      DiscoveryFilterType.COMMISSIONING_MODE, DiscoveryFilterType.COMMISSIONER -> {}
      DiscoveryFilterType.DEVICE_TYPE -> addArgument(
        "device-type",
        0,
        Long.MAX_VALUE,
        discoveryFilterCode,
        null,
        false
      )

      DiscoveryFilterType.INSTANCE_NAME -> addArgument("name", discoveryFilterInstanceName, null, false)
    }
    addArgument("timeout", 0L, Long.MAX_VALUE, timeoutMillis, null, false)
  }

  override fun onConnectDeviceComplete() {
    logger.log(Level.INFO, "onConnectDeviceComplete")
  }

  override fun onStatusUpdate(status: Int) {
    logger.log(Level.INFO, "onStatusUpdate with status: $status")
  }

  override fun onPairingComplete(errorCode: Int) {
    logger.log(Level.INFO, "onPairingComplete with error code: $errorCode")
    if (errorCode != 0) {
      setFailure("onPairingComplete failure")
    }
  }

  override fun onPairingDeleted(errorCode: Int) {
    logger.log(Level.INFO, "onPairingDeleted with error code: $errorCode")
  }

  override fun onCommissioningComplete(nodeId: Long, errorCode: Int) {
    logger.log(Level.INFO, "onCommissioningComplete with error code: $errorCode")
    if (errorCode == 0) {
      setSuccess()
    } else {
      setFailure("onCommissioningComplete failure")
    }
  }

  override fun onReadCommissioningInfo(
    vendorId: Int, productId: Int, wifiEndpointId: Int, threadEndpointId: Int
  ) {
    logger.log(Level.INFO, "onReadCommissioningInfo")
  }

  override fun onCommissioningStatusUpdate(nodeId: Long, stage: String?, errorCode: Int) {
    logger.log(Level.INFO, "onCommissioningStatusUpdate")
  }

  override fun onNotifyChipConnectionClosed() {
    logger.log(Level.INFO, "onNotifyChipConnectionClosed")
  }

  override fun onCloseBleComplete() {
    logger.log(Level.INFO, "onCloseBleComplete")
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

  fun getNodeId(): Long {
    return nodeId.get()
  }

  fun getRemoteAddr(): IPAddress {
    return remoteAddr!!
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
}