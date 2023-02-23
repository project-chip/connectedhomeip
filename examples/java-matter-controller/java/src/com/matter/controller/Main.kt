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
import com.matter.controller.commands.pairing.*
import java.util.ArrayList

private fun registerCommandsDiscover(
  controller: ChipDeviceController,
  commandManager: CommandManager,
  credentialsIssuer: CredentialsIssuer
) {
  val clusterCommands: ArrayList<Command> = ArrayList<Command>()
  val discoverCommand = DiscoverCommand(controller, credentialsIssuer)
  val discoverCommissionablesCommand = DiscoverCommissionablesCommand(controller, credentialsIssuer)
  val discoverCommissionersCommand = DiscoverCommissionersCommand(controller, credentialsIssuer)
  clusterCommands.add(discoverCommand)
  clusterCommands.add(discoverCommissionablesCommand)
  clusterCommands.add(discoverCommissionersCommand)
  commandManager.register("discover", clusterCommands)
}

private fun registerCommandsPairing(
  controller: ChipDeviceController,
  commandManager: CommandManager,
  credentialsIssuer: CredentialsIssuer
) {
  val clusterCommands: ArrayList<Command> = ArrayList<Command>()
  val unpairCommand = UnpairCommand(controller, credentialsIssuer)
  val pairCodeCommand = PairCodeCommand(controller, credentialsIssuer)
  val pairCodePaseCommand = PairCodePaseCommand(controller, credentialsIssuer)
  val pairCodeWifiCommand = PairCodeWifiCommand(controller, credentialsIssuer)
  val pairCodeThreadCommand = PairCodeThreadCommand(controller, credentialsIssuer)
  val pairAddressPaseCommand = PairAddressPaseCommand(controller, credentialsIssuer)
  val pairAlreadyDiscoveredCommand = PairAlreadyDiscoveredCommand(controller, credentialsIssuer)
  val pairOnNetworkCommand = PairOnNetworkCommand(controller, credentialsIssuer)
  val pairOnNetworkShortCommand = PairOnNetworkShortCommand(controller, credentialsIssuer)
  val pairOnNetworkLongCommand = PairOnNetworkLongCommand(controller, credentialsIssuer)
  val pairOnNetworkVendorCommand = PairOnNetworkVendorCommand(controller, credentialsIssuer)
  val pairOnNetworkCommissioningModeCommand = PairOnNetworkCommissioningModeCommand(controller, credentialsIssuer)
  val pairOnNetworkCommissionerCommand = PairOnNetworkCommissionerCommand(controller, credentialsIssuer)
  val pairOnNetworkDeviceTypeCommand = PairOnNetworkDeviceTypeCommand(controller, credentialsIssuer)
  val pairOnNetworkInstanceNameCommand = PairOnNetworkInstanceNameCommand(controller, credentialsIssuer)
  clusterCommands.add(unpairCommand)
  clusterCommands.add(pairCodeCommand)
  clusterCommands.add(pairCodePaseCommand)
  clusterCommands.add(pairCodeWifiCommand)
  clusterCommands.add(pairCodeThreadCommand)
  clusterCommands.add(pairAddressPaseCommand)
  clusterCommands.add(pairAlreadyDiscoveredCommand)
  clusterCommands.add(pairOnNetworkCommand)
  clusterCommands.add(pairOnNetworkShortCommand)
  clusterCommands.add(pairOnNetworkLongCommand)
  clusterCommands.add(pairOnNetworkVendorCommand)
  clusterCommands.add(pairOnNetworkCommissioningModeCommand)
  clusterCommands.add(pairOnNetworkCommissionerCommand)
  clusterCommands.add(pairOnNetworkDeviceTypeCommand)
  clusterCommands.add(pairOnNetworkInstanceNameCommand)
  commandManager.register("pairing", clusterCommands)
}

fun main(args: Array<String>) {
  val controller = ChipDeviceController(
    ControllerParams.newBuilder()
      .setUdpListenPort(0)
      .setControllerVendorId(0xFFF1)
      .setCountryCode("US")
      .build()
  )
  val credentialsIssuer = CredentialsIssuer()
  val commandManager = CommandManager()
  registerCommandsDiscover(controller, commandManager, credentialsIssuer)
  registerCommandsPairing(controller, commandManager, credentialsIssuer)
  try {
    commandManager.run(args)
  } catch (e: Exception) {
    println("Run command failed with exception: " + e.message)
    System.exit(1)
  }
  controller.shutdownCommissioning()
}