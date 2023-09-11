/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.controller.commands.discover

import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.DiscoveredDevice
import com.matter.controller.commands.common.CredentialsIssuer
import com.matter.controller.commands.common.MatterCommand
import java.util.concurrent.TimeUnit

class DiscoverCommissionablesCommand(
  controller: ChipDeviceController,
  credsIssuer: CredentialsIssuer?
) : MatterCommand(controller, credsIssuer, "commissionables") {
  override fun runCommand() {
    currentCommissioner().discoverCommissionableNodes()

    try {
      TimeUnit.SECONDS.sleep(TIME_TO_WAIT_FOR_RESULTS_SECONDS)
    } catch (e: InterruptedException) {
      throw RuntimeException(e)
    }

    logDiscoveredDevice()
  }

  private fun logDiscoveredDevice() {
    // Log at most MAX_DISCOVERED_DEVICES discovered devices
    for (i in 0 until MAX_DISCOVERED_DEVICES) {
      val device: DiscoveredDevice = currentCommissioner().getDiscoveredDevice(i) ?: break
      logDevice(device)
    }
  }

  private fun logDevice(device: DiscoveredDevice) {
    println("Discovered node:")
    System.out.format("\tDiscriminator: %d", device.discriminator)
    System.out.format("\tIP Address : %s%n", device.ipAddress)
  }

  companion object {
    private const val MAX_DISCOVERED_DEVICES = 10
    private const val TIME_TO_WAIT_FOR_RESULTS_SECONDS = 7L
  }
}
