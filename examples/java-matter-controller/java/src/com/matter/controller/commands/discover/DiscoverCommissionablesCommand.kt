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
