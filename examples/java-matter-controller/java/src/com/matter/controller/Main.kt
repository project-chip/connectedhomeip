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
package com.matter.controller

import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ControllerParams
import com.matter.controller.commands.common.*
import com.matter.controller.commands.discover.*
import com.matter.controller.commands.icd.*
import com.matter.controller.commands.pairing.*

private fun getDiscoveryCommands(
  controller: ChipDeviceController,
  credentialsIssuer: CredentialsIssuer
): List<Command> {
  return listOf(
    DiscoverCommand(controller, credentialsIssuer),
    DiscoverCommissionablesCommand(controller, credentialsIssuer),
    DiscoverCommissionersCommand(controller, credentialsIssuer),
  )
}

private fun getPairingCommands(
  controller: ChipDeviceController,
  credentialsIssuer: CredentialsIssuer
): List<Command> {
  return listOf(
    UnpairCommand(controller, credentialsIssuer),
    PairCodeCommand(controller, credentialsIssuer),
    PairCodePaseCommand(controller, credentialsIssuer),
    PairCodeWifiCommand(controller, credentialsIssuer),
    PairCodeThreadCommand(controller, credentialsIssuer),
    PairAddressPaseCommand(controller, credentialsIssuer),
    PairAlreadyDiscoveredCommand(controller, credentialsIssuer),
    PairOnNetworkCommand(controller, credentialsIssuer),
    PairOnNetworkShortCommand(controller, credentialsIssuer),
    PairOnNetworkLongCommand(controller, credentialsIssuer),
    PairOnNetworkVendorCommand(controller, credentialsIssuer),
    PairOnNetworkCommissioningModeCommand(controller, credentialsIssuer),
    PairOnNetworkCommissionerCommand(controller, credentialsIssuer),
    PairOnNetworkDeviceTypeCommand(controller, credentialsIssuer),
    PairOnNetworkInstanceNameCommand(controller, credentialsIssuer),
  )
}

private fun getImCommands(
  controller: ChipDeviceController,
  credentialsIssuer: CredentialsIssuer
): List<Command> {
  return listOf(
    PairOnNetworkLongImReadCommand(controller, credentialsIssuer),
    PairOnNetworkLongImSubscribeCommand(controller, credentialsIssuer),
    PairOnNetworkLongImWriteCommand(controller, credentialsIssuer),
    PairOnNetworkLongImInvokeCommand(controller, credentialsIssuer),
    PairOnNetworkLongImExtendableInvokeCommand(controller, credentialsIssuer),
  )
}

private fun getICDCommands(
  controller: ChipDeviceController,
  credentialsIssuer: CredentialsIssuer
): List<Command> {
  return listOf(
    ICDListCommand(controller, credentialsIssuer),
  )
}

fun main(args: Array<String>) {
  val controller =
    ChipDeviceController(
      ControllerParams.newBuilder()
        .setUdpListenPort(0)
        .setControllerVendorId(0xFFF1)
        .setCountryCode("US")
        .build()
    )
  val credentialsIssuer = CredentialsIssuer()
  val commandManager = CommandManager()

  commandManager.register("discover", getDiscoveryCommands(controller, credentialsIssuer))
  commandManager.register("pairing", getPairingCommands(controller, credentialsIssuer))
  commandManager.register("im", getImCommands(controller, credentialsIssuer))
  commandManager.register("icd", getICDCommands(controller, credentialsIssuer))

  try {
    commandManager.run(args)
  } catch (e: Exception) {
    println("Run command failed with exception: " + e.message)
    System.exit(1)
  }

  controller.shutdownCommissioning()
}
